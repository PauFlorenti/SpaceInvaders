#include <renderer/buffer.h>
#include <renderer/render_types.h>

Buffer create_buffer(
	VulkanState* state,
	size_t size,
	VkBufferUsageFlags usage_flags,
	VmaMemoryUsage memory_usage)
{

	VkBufferCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage_flags
	};

	VmaAllocationCreateInfo alloc_info{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memory_usage,
	};

	Buffer buffer;
	VKCHECK(vmaCreateBuffer(state->allocator, &info, &alloc_info, &buffer.buffer, &buffer.allocation, &buffer.allocation_info));
	return buffer;
}

void destroy_buffer(VulkanState* state, const Buffer& buffer)
{
	vmaDestroyBuffer(state->allocator, buffer.buffer, buffer.allocation);
}