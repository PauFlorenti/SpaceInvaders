#include <renderer/buffer.h>
#include <renderer/render_types.h>
#include <renderer/utils.h>

#define VMA_IMPLEMENTATION
#pragma warning(disable : 4324)
#pragma warning(disable : 4189)
#pragma warning(disable : 4127)
#pragma warning(disable : 4100)
#include <vma/vk_mem_alloc.h>
#pragma warning(default : 4100)
#pragma warning(default : 4127)
#pragma warning(default : 4189)
#pragma warning(default : 4324)

void immediate_submit(VulkanState* state, std::function<void(VkCommandBuffer cmd)>&& function)
{
	const auto& device = state->device;
	VKCHECK(vkResetFences(device, 1, &state->immediate_submit_fence));
	VKCHECK(vkResetCommandBuffer(state->immediate_submit_command_buffer, 0));

	auto cmd = state->immediate_submit_command_buffer;
	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VKCHECK(vkBeginCommandBuffer(cmd, &begin_info));

	function(cmd);

	VKCHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo submit_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
	};

	VkSubmitInfo submit{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
	};

	VKCHECK(vkQueueSubmit(state->graphics_queue, 1, &submit, state->immediate_submit_fence));
	VKCHECK(vkWaitForFences(device, 1, &state->immediate_submit_fence, VK_TRUE, UINT64_MAX));

}

bool load_shader_module(const char* filename, VkDevice device, VkShaderModule* out_shader_module)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	const size_t file_size = static_cast<size_t>(file.tellg());
	std::vector<uint32_t>buffer(file_size / sizeof(uint32_t));

	file.seekg(0);
	file.read((char*)buffer.data(), file_size);
	file.close();

	VkShaderModuleCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = buffer.size() * sizeof(uint32_t),
		.pCode = buffer.data(),
	};

	VkShaderModule module;
	if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
		return false;

	*out_shader_module = module;
	return true;
}

void upload_mesh_utils(VulkanState* state, Mesh& mesh)
{
	assert(!mesh.vertices.empty());
	assert(!mesh.indices.empty());

	const size_t vertex_buffer_size = mesh.vertices.size() * sizeof(Vertex);
	const size_t index_buffer_size = mesh.indices.size() * sizeof(uint16_t);

	mesh.vertex_buffer = create_buffer(state, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	mesh.index_buffer = create_buffer(state, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	Buffer staging = create_buffer(state, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data = staging.allocation->GetMappedData();

	memcpy(data, mesh.vertices.data(), vertex_buffer_size);
	memcpy((char*)data + vertex_buffer_size, mesh.indices.data(), index_buffer_size);

	immediate_submit(state, [&](VkCommandBuffer cmd) {
		VkBufferCopy vertex_copy{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = vertex_buffer_size,
		};

		vkCmdCopyBuffer(cmd, staging.buffer, mesh.vertex_buffer.buffer, 1, &vertex_copy);

		VkBufferCopy index_copy{
			.srcOffset = vertex_buffer_size,
			.dstOffset = 0,
			.size = index_buffer_size,
		};

		vkCmdCopyBuffer(cmd, staging.buffer, mesh.index_buffer.buffer, 1, &index_copy);
		});

	destroy_buffer(state, staging);
}

void destroy_mesh_utils(VulkanState* state, Mesh& mesh)
{
	destroy_buffer(state, mesh.vertex_buffer);
	destroy_buffer(state, mesh.index_buffer);
}
