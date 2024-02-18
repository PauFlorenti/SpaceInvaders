#pragma once

#include <defines.h>
#include <vulkan/vulkan.h>

#pragma warning(disable : 4324)
#pragma warning(disable : 4189)
#pragma warning(disable : 4127)
#pragma warning(disable : 4100)
#include <vma/vk_mem_alloc.h>
#pragma warning(default : 4100)
#pragma warning(default : 4127)
#pragma warning(default : 4189)
#pragma warning(default : 4324)

struct VulkanState;

struct Texture
{
	VkImage image = VK_NULL_HANDLE;
	VkImageView image_view = VK_NULL_HANDLE;
	VmaAllocation allocation;
	VkExtent2D extent{};
	VkFormat format = VK_FORMAT_UNDEFINED;
};

Texture create_texture(
	VulkanState* state,
	VkExtent3D extent,
	VkFormat format,
	VkImageUsageFlags usage_flags,
	VkImageAspectFlags aspect_flags,
	void* data = nullptr);

void destroy_texture(VulkanState* state, Texture& texture);

// Faster than copy blit image, but restricted to match resolution.
void copy_image_to_image(
	VkCommandBuffer cmd,
	VkImage src,
	VkImageLayout src_layout,
	VkImage dst,
	VkImageLayout dst_layout,
	VkExtent3D extent);

// Slower than standard copy image, but format and size does not need to match.
void copy_blit_image_to_image(
	VkCommandBuffer cmd,
	VkImage src,
	VkImage dst,
	VkExtent2D src_extent,
	VkExtent2D dst_extent);

void transition_image(
	VkCommandBuffer cmd, 
	VkImage image, 
	VkImageLayout initial_layout, 
	VkImageLayout final_layout);