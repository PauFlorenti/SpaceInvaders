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

struct Buffer
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation;
	VmaAllocationInfo allocation_info;
	VkDeviceAddress address;
};

Buffer create_buffer(
	VulkanState* state,
	size_t allocation_size,
	VkBufferUsageFlags usage_flags,
	VmaMemoryUsage memory_usage);

void destroy_buffer(VulkanState* state, const Buffer& buffer);