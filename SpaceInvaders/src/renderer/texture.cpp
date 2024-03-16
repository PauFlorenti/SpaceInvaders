#include <renderer/buffer.h>
#include <renderer/render_types.h>
#include <renderer/texture.h>
#include <renderer/utils.h>

Texture create_texture(
	VulkanState* state,
	VkExtent3D extent,
	VkFormat format,
	VkImageUsageFlags usage_flags,
	VkImageAspectFlags aspect_flags,
	void* data)
{
	Texture t;
	t.extent = {extent.width, extent.height};

	VkImageUsageFlags image_usage_flags = usage_flags;
	if (data)
	{
		image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkImageCreateInfo image_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = image_usage_flags,
	};

	VmaAllocationCreateInfo allocation_info{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};

	VKCHECK(vmaCreateImage(state->allocator, &image_info, &allocation_info, &t.image, &t.allocation, nullptr));

	VkImageViewCreateInfo view_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = t.image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = aspect_flags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.layerCount = 1,
		},
	};

	VKCHECK(vkCreateImageView(state->device, &view_info, nullptr, &t.image_view));

	if (data)
	{
		const size_t size = extent.width * extent.height * 4;
		Buffer staging_buffer = create_buffer(state, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		memcpy(staging_buffer.allocation_info.pMappedData, data, size);

		immediate_submit(state, [&](VkCommandBuffer cmd) {
			transition_image(cmd, t.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy region{
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
				.imageExtent = extent,
			};

			vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			transition_image(cmd, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

		destroy_buffer(state, staging_buffer);
	}

	return t;
}

void destroy_texture(VulkanState* state, Texture& texture)
{
	vkDestroyImageView(state->device, texture.image_view, nullptr);
	vmaDestroyImage(state->allocator, texture.image, texture.allocation);
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, VkExtent3D extent)
{
	VkImageSubresourceLayers src_subresource{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageSubresourceLayers dst_subresource{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageCopy region{
		.srcSubresource = src_subresource,
		.srcOffset = 0,
		.dstSubresource = dst_subresource,
		.dstOffset = 0,
		.extent = extent,
	};

	vkCmdCopyImage(cmd, src, src_layout, dst, dst_layout, 1, &region);
}

void copy_blit_image_to_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_extent, VkExtent2D dst_extent)
{
	VkImageBlit2 blit_region{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.srcSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.dstSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	blit_region.srcOffsets[1] = {
		.x = (int32_t)src_extent.width,
		.y = (int32_t)src_extent.height,
		.z = 1,
	};

	blit_region.dstOffsets[1] = {
		.x = (int32_t)dst_extent.width,
		.y = (int32_t)dst_extent.height,
		.z = 1,
	};

	VkBlitImageInfo2 blit_info = {
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.srcImage = src,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = dst,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blit_region,
		.filter = VK_FILTER_LINEAR,
	};

	vkCmdBlitImage2(cmd, &blit_info);
}

void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout initial_layout, VkImageLayout final_layout)
{
	VkImageAspectFlags mask = final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = initial_layout,
		.newLayout = final_layout,
		.image = image,
		.subresourceRange = {
			.aspectMask = mask,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	VkDependencyInfo dependency_info{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	vkCmdPipelineBarrier2(cmd, &dependency_info);
}
