
#include <camera.h>
#include <renderer/buffer.h>
#include <renderer/render.h>
#include <renderer/render_types.h>
#include <renderer/texture.h>
#include <renderer/utils.h>
#include <scene.h>

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(disable : 4244)
#include <stb/stb_image.h>
#pragma warning(default : 4244)

#include <vulkan-bootstrap/VkBootstrap.h>

#ifdef _DEBUG
bool reques_validation_layers = true;
#else
bool reques_validation_layers = false;
#endif

static VulkanState* state = nullptr;

static bool vulkan_init()
{
	vkb::InstanceBuilder instance_builder;

	auto instance_result = instance_builder
		.set_app_name("SpaceInvaders")
		.request_validation_layers(reques_validation_layers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	if (!instance_result)
	{
		printf("[ERROR]: %s", instance_result.error().message().c_str());
		return false;
	}

	vkb::Instance vkb_instance = instance_result.value();
	state->instance = vkb_instance.instance;

#ifdef _DEBUG
	state->debug_messenger = vkb_instance.debug_messenger;
#endif

	const auto error = glfwCreateWindowSurface(state->instance, state->window, nullptr, &state->surface);
	if (error != VK_SUCCESS)
	{
		const char* error_msg;
		int error_value = glfwGetError(&error_msg);
		if (error_value != 0)
		{
			printf("%s\n", error_msg);
		}
		return false;
	}

	VkPhysicalDeviceVulkan12Features features_12{
		.bufferDeviceAddress = true,
	};

	VkPhysicalDeviceVulkan13Features features_13{
		.synchronization2 = true,
		.dynamicRendering = true,
	};

	vkb::PhysicalDeviceSelector gpu_selector{ vkb_instance };
	auto gpu_result = gpu_selector
		.set_minimum_version(1, 1)
		.set_required_features_12(features_12)
		.set_required_features_13(features_13)
		.set_surface(state->surface)
		.select();

	if (!gpu_result)
	{
		printf("[ERROR]: %s", gpu_result.error().message().c_str());
		return false;
	}

	auto physical_device = gpu_result.value();

	vkb::DeviceBuilder device_builder{ physical_device };
	vkb::Device vkb_device = device_builder.build().value();

	state->device = vkb_device.device;
	state->gpu = vkb_device.physical_device;
	state->graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	state->graphics_queue_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocator_info{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = state->gpu,
		.device = state->device,
		.instance = state->instance,
	};

	VKCHECK(vmaCreateAllocator(&allocator_info, &state->allocator));

	return true;
}

static bool create_swapchain(int32_t width, int32_t height)
{
	vkb::SwapchainBuilder builder{ state->gpu, state->device, state->surface };
	auto swapchain_result = builder
		.set_desired_extent(width, height)
		.set_desired_format({ .format = state->swapchain.format, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR })
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build();

	if (!swapchain_result)
	{
		printf("[ERROR]: %s", swapchain_result.error().message().c_str());
		return false;
	}

	if (!swapchain_result.has_value())
		return false;

	auto swpch = swapchain_result.value();

	auto& swapchain = state->swapchain;
	swapchain.extent = swpch.extent;
	swapchain.swapchain = swpch.swapchain;
	swapchain.images = swpch.get_images().value();
	swapchain.image_views = swpch.get_image_views().value();

	return true;
}

static bool init_commands()
{
	VkCommandPoolCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = state->graphics_queue_index,
	};

	// Per frame commands
	for (int i = 0; i < MAX_FRAME_COUNT; ++i)
	{
		auto ok = vkCreateCommandPool(state->device, &info, nullptr, &state->frame_data[i].command_pool);
		if (ok != VK_SUCCESS)
		{
			printf("[ERROR]: Failed to create command pool.");
			return false;
		}

		VkCommandBufferAllocateInfo allocate_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = state->frame_data[i].command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		ok = vkAllocateCommandBuffers(state->device, &allocate_info, &state->frame_data[i].command_buffer);
		if (ok != VK_SUCCESS)
		{
			printf("[ERROR]: Failed to create command buffer.");
			return false;
		}
	}

	// Immediate submit commands
	VKCHECK(vkCreateCommandPool(state->device, &info, nullptr, &state->immediate_submit_command_pool));

	VkCommandBufferAllocateInfo immediate_submit_cmd_allocate_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = state->immediate_submit_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VKCHECK(vkAllocateCommandBuffers(state->device, &immediate_submit_cmd_allocate_info, &state->immediate_submit_command_buffer));

	return true;
}

static bool init_sync_objects()
{
	VkFenceCreateInfo fence_info{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (int i = 0; i < MAX_FRAME_COUNT; ++i)
	{
		auto& frame = state->frame_data[i];

		if (vkCreateFence(state->device, &fence_info, nullptr, &frame.fence) != VK_SUCCESS)
			return false;

		VkSemaphoreCreateInfo semaphore_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		if (vkCreateSemaphore(state->device, &semaphore_info, nullptr, &frame.present_semaphore) != VK_SUCCESS)
			return false;

		if (vkCreateSemaphore(state->device, &semaphore_info, nullptr, &frame.render_semaphore) != VK_SUCCESS)
			return false;
	}

	VKCHECK(vkCreateFence(state->device, &fence_info, nullptr, &state->immediate_submit_fence));

	return true;
}

static void init_descriptors()
{
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
	};

	for (uint32_t i = 0; i < MAX_FRAME_COUNT; ++i)
		state->frame_data[i].descriptor_allocator->init(state->device, 10, sizes);

	DescriptorLayoutBuilder builder;
	builder.clear();
	builder.add_bindings(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state->material_set_layout = builder.build(state->device, VK_SHADER_STAGE_FRAGMENT_BIT);
}

static void init_forward_pipelines()
{
	VkShaderModule vertex_shader;
	if (!load_shader_module("SpaceInvaders/shaders/bin/forward.vert.spv", state->device, &vertex_shader))
	{
		printf("[ERROR]: Error building the forward vertex shader.");
	}

	VkShaderModule fragment_shader;
	if (!load_shader_module("SpaceInvaders/shaders/bin/forward.frag.spv", state->device, &fragment_shader))
	{
		printf("[ERROR]: Error building the forward fragment shader.");
	}

	std::vector<VkVertexInputAttributeDescription> input_attributes{
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, uv),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, normal),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Vertex, color),
		}
	};

	VkPushConstantRange push_constant_range{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(VertexPushConstant),
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &state->material_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range,
	};

	VKCHECK(vkCreatePipelineLayout(state->device, &layout_info, nullptr, &state->forward_pipeline_layout));

	PipelineBuilder builder;
	builder.layout = state->forward_pipeline_layout;
	builder.set_shaders(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
	builder.set_shaders(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	builder.set_input_attribute(std::move(input_attributes), sizeof(Vertex));
	builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	builder.set_multisampling_none();
	builder.disable_blending();
	builder.disable_depth_test();

	builder.set_color_attachment_format(state->draw_image.format);
	builder.set_depth_format(VK_FORMAT_UNDEFINED);

	state->forward_pipeline = builder.build(state->device);

	vkDestroyShaderModule(state->device, vertex_shader, nullptr);
	vkDestroyShaderModule(state->device, fragment_shader, nullptr);
}

static void init_pipelines()
{
	init_forward_pipelines();
}

static void destroy_swapchain()
{
	auto& swapchain = state->swapchain;

	for (auto& view : swapchain.image_views)
	{
		vkDestroyImageView(state->device, view, nullptr);
	}

	vkDestroySwapchainKHR(state->device, swapchain.swapchain, nullptr);
}

static void resize_swapchain()
{
	VKCHECK(vkDeviceWaitIdle(state->device));

	destroy_swapchain();
	destroy_texture(state, state->draw_image);

	int32_t w, h;
	glfwGetWindowSize(state->window, &w, &h);

	create_swapchain(w, h);

	VkExtent3D draw_image_extent = { state->swapchain.extent.width, state->swapchain.extent.height, 1 };
	state->draw_image = create_texture(
		state,
		draw_image_extent,
		state->swapchain.format,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	state->resize_requested = false;
}

bool render_init(GLFWwindow* window)
{
	assert(window != nullptr);
	state = new VulkanState();
	state->window = window;

	if (!vulkan_init())
		return false;

	int32_t w, h;
	glfwGetWindowSize(state->window, &w, &h);

	if (!create_swapchain(w, h))
		return false;

	if (!init_commands())
		return false;

	if (!init_sync_objects())
		return false;

	VkExtent3D draw_image_extent = { state->swapchain.extent.width, state->swapchain.extent.height, 1 };
	state->draw_image = create_texture(
		state,
		draw_image_extent,
		state->swapchain.format,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	init_descriptors();
	init_pipelines();

	VkSamplerCreateInfo sampler_info{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
	};

	vkCreateSampler(state->device, &sampler_info, nullptr, &state->sampler);

	return true;
}

bool begin_draw()
{
	state->frame_index = state->frame_count % MAX_FRAME_COUNT;
	auto& frame = state->frame_data[state->frame_index];
	VKCHECK(vkWaitForFences(state->device, 1, &frame.fence, VK_TRUE, UINT64_MAX));
	VKCHECK(vkResetFences(state->device, 1, &frame.fence));

	state->frame_data[state->frame_index].descriptor_allocator->clear(state->device);

	if (state->resize_requested)
	{
		int32_t w = 0, h = 0;
		glfwGetWindowSize(state->window, &w, &h);

		while (w == 0 && h == 0)
		{
			glfwGetWindowSize(state->window, &w, &h);
			glfwWaitEvents();
		}

		resize_swapchain();
	}

	auto ok = vkAcquireNextImageKHR(state->device, state->swapchain.swapchain, UINT64_MAX, frame.present_semaphore, nullptr, &state->swapchain.image_index);
	if (ok != VK_SUCCESS)
	{
		if (ok == VK_ERROR_OUT_OF_DATE_KHR)
		{
			state->resize_requested = true;
			return false;
		}
		else
		{
			throw std::runtime_error("Failed to acquire next image.");
		}
	}

	VKCHECK(vkResetCommandBuffer(frame.command_buffer, 0));

	VkCommandBufferBeginInfo cmd_begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	VKCHECK(vkBeginCommandBuffer(frame.command_buffer, &cmd_begin_info));

	return true;
}

void draw(const Scene& scene, const Camera& camera)
{
	const auto cmd = state->frame_data[state->frame_index].command_buffer;
	const auto swapchain_image = state->swapchain.images[state->swapchain.image_index];

	// Clear background
	transition_image(cmd, state->draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	VkClearColorValue clear_color{};
	clear_color.float32[1] = 1.0f;
	VkImageSubresourceRange region{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	vkCmdClearColorImage(cmd, state->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &region);

	transition_image(cmd, state->draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	// Draw geometry
	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = state->draw_image.image_view,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	};

	VkExtent2D image_extent = {
		.width = std::min(state->swapchain.extent.width, state->draw_image.extent.width),
		.height = std::min(state->swapchain.extent.height, state->draw_image.extent.height),
	};

	VkRenderingInfo rendering_info{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.offset = {0, 0},
			.extent = image_extent,
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
	};

	vkCmdBeginRendering(cmd, &rendering_info);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state->forward_pipeline);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = (float)state->swapchain.extent.height;
	viewport.width = (float)state->swapchain.extent.width;
	viewport.height = -(float)state->swapchain.extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { state->swapchain.extent.width, state->swapchain.extent.height };

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// Draw enemies
	Material* current_material = nullptr;
	for (const auto e : scene.entities)
	{
		VertexPushConstant pc;
		pc.model = glm::translate(glm::mat4(1.0f), glm::vec3(e->position)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
		pc.view_projection = camera.get_view_projection();

		vkCmdPushConstants(cmd, state->forward_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstant), &pc);

		if (current_material != e->material)
		{
			VkDescriptorSet enemy_material_set = state->frame_data[state->frame_index].descriptor_allocator->allocate(state->device, state->material_set_layout);
			{
				VkDescriptorImageInfo image_info{
				.sampler = state->sampler,
				.imageView = e->material->albedo->image_view,
				.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
				};

				VkWriteDescriptorSet write{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = enemy_material_set,
					.dstBinding = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &image_info,
				};

				vkUpdateDescriptorSets(state->device, 1, &write, 0, VK_NULL_HANDLE);
			}

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state->forward_pipeline_layout, 0, 1, &enemy_material_set, 0, VK_NULL_HANDLE);
			current_material = e->material;
		}


		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &e->mesh->vertex_buffer.buffer, &offset);
		vkCmdBindIndexBuffer(cmd, e->mesh->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd, (uint32_t)e->mesh->indices.size(), 1, 0, 0, 0);
	}

	vkCmdEndRendering(cmd);

	// Copy image to swapchain.
	transition_image(cmd, state->draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copy_image_to_image(cmd, state->draw_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { state->swapchain.extent.width, state->swapchain.extent.height, 1 });

	transition_image(cmd, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void end_draw()
{
	const auto& frame = state->frame_data[state->frame_index];

	VKCHECK(vkEndCommandBuffer(frame.command_buffer));

	VkSubmitInfo submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.present_semaphore,
		.commandBufferCount = 1,
		.pCommandBuffers = &frame.command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &frame.render_semaphore,
	};

	VKCHECK(vkQueueSubmit(state->graphics_queue, 1, &submit_info, frame.fence));

	VkPresentInfoKHR present_info{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.render_semaphore,
		.swapchainCount = 1,
		.pSwapchains = &state->swapchain.swapchain,
		.pImageIndices = &state->swapchain.image_index,
	};

	auto ok = vkQueuePresentKHR(state->graphics_queue, &present_info);
	if (ok == VK_ERROR_OUT_OF_DATE_KHR)
		state->resize_requested = true;

	++state->frame_count;
}

void upload_mesh(Mesh& mesh)
{
	upload_mesh_utils(state, mesh);
}

void destroy_mesh(Mesh& mesh)
{
	destroy_mesh_utils(state, mesh);
}

Texture* create_texture(const std::string texture)
{
	int32_t width, height, channels;
	stbi_uc* pixels = stbi_load(texture.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error("Failed to load texture.");

	Texture* t = new Texture();

	VkExtent3D extent = { (uint32_t)width, (uint32_t)height, 1 };
	*t = create_texture(
		state,
		extent,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		pixels);

	stbi_image_free(pixels);

	return t;
}

void destroy_texture(Texture& texture)
{
	destroy_texture(state, texture);
}

void render_wait_idle()
{
	VKCHECK(vkDeviceWaitIdle(state->device));
}

void render_shutdown()
{
	auto device = state->device;

	VKCHECK(vkDeviceWaitIdle(device));

	destroy_texture(state, state->draw_image);
	vkDestroySampler(device, state->sampler, nullptr);

	vkDestroyPipeline(device, state->forward_pipeline, nullptr);
	vkDestroyPipelineLayout(device, state->forward_pipeline_layout, nullptr);

	vkDestroyDescriptorSetLayout(device, state->material_set_layout, nullptr);

	destroy_swapchain();

	for (int i = 0; i < MAX_FRAME_COUNT; ++i)
	{
		state->frame_data[i].descriptor_allocator->destroy(device);

		auto& frame = state->frame_data[i];
		vkDestroyCommandPool(device, frame.command_pool, nullptr);
		vkDestroyFence(device, frame.fence, nullptr);
		vkDestroySemaphore(device, frame.present_semaphore, nullptr);
		vkDestroySemaphore(device, frame.render_semaphore, nullptr);
	}

	vkDestroyCommandPool(device, state->immediate_submit_command_pool, nullptr);
	vkDestroyFence(device, state->immediate_submit_fence, nullptr);

	vmaDestroyAllocator(state->allocator);

	vkDestroyDevice(state->device, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);

#ifdef _DEBUG
	vkb::destroy_debug_utils_messenger(state->instance, state->debug_messenger);
#endif // DEBUG

	vkDestroyInstance(state->instance, nullptr);

	delete state;
}