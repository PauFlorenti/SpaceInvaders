#pragma once

#include <defines.h>

#include <renderer/buffer.h>
#include <renderer/texture.h>

#include <vulkan/vk_enum_string_helper.h>

#pragma warning(disable : 4324)
#pragma warning(disable : 4189)
#pragma warning(disable : 4127)
#pragma warning(disable : 4100)
#include <vma/vk_mem_alloc.h>
#pragma warning(default : 4100)
#pragma warning(default : 4127)
#pragma warning(default : 4189)
#pragma warning(default : 4324)

constexpr uint32_t MAX_FRAME_COUNT = 3;

#define VKCHECK(x)												    \
{																    \
	VkResult err = x;											    \
	if (err)													    \
	{															    \
		printf("Vulkan command error: {%s}", string_VkResult(err)); \
		abort();													\
	}																\
}

struct Vertex
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec4 color;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	Buffer vertex_buffer;
	Buffer index_buffer;
	std::string name;
};

struct Material
{
	Texture* albedo = nullptr;
	glm::vec4 color = glm::vec4(1.0f);
};

struct VertexPushConstant
{
	glm::mat4 model;
	glm::mat4 view_projection;
};

struct DescriptorLayoutBuilder
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void add_bindings(uint32_t binding, VkDescriptorType type);
	void clear();
	VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages);
};

struct DescriptorAllocator
{
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool pool;

	void init(VkDevice device, uint32_t max_sets, const std::span<PoolSizeRatio>& ratios);
	void clear(VkDevice device);
	void destroy(VkDevice device);

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

struct PipelineBuilder
{
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout layout;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineRenderingCreateInfo render_info;
	VkFormat color_attachment_format;

	PipelineBuilder() { clear(); }
	void clear();
	void set_shaders(VkShaderModule shader_module, VkShaderStageFlags shader_stage);
	void set_topology(VkPrimitiveTopology topology);
	void set_polygon_mode(VkPolygonMode polygon_mode);
	void set_cull_mode(VkCullModeFlags cull_mode_flags, VkFrontFace front_face);
	void set_multisampling_none();
	void disable_blending();
	void set_color_attachment_format(VkFormat format);
	void set_depth_format(VkFormat format);
	void disable_depth_test();
	VkPipeline build(VkDevice device);
};

struct Swapchain
{
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkExtent2D extent = { 0, 0 };
	std::vector<VkImage> images;
	std::vector<VkImageView> image_views;
	uint32_t image_index;
};

struct FrameData
{
	VkSemaphore present_semaphore = VK_NULL_HANDLE;
	VkSemaphore render_semaphore = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	DescriptorAllocator descriptor_allocator[MAX_FRAME_COUNT];
};

struct VulkanState
{
	GLFWwindow* window = nullptr;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	Swapchain swapchain;
	FrameData frame_data[MAX_FRAME_COUNT];
	uint32_t frame_index = 0;
	uint32_t frame_count = 0;
	bool resize_requested{ false };

	VkQueue graphics_queue;
	uint32_t graphics_queue_index;

	VmaAllocator allocator;

	VkCommandPool immediate_submit_command_pool;
	VkCommandBuffer immediate_submit_command_buffer;
	VkFence immediate_submit_fence;

	VkDescriptorSetLayout material_set_layout = VK_NULL_HANDLE;

	VkSampler sampler;

	VkPipeline forward_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout forward_pipeline_layout = VK_NULL_HANDLE;

	Texture draw_image;

#ifdef _DEBUG
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
#endif
};