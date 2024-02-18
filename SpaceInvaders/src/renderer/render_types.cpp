#include <renderer/render_types.h>

void DescriptorLayoutBuilder::add_bindings(uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding layout_binding{
		.binding = binding,
		.descriptorType = type,
		.descriptorCount = 1,
	};

	bindings.push_back(layout_binding);
}

void DescriptorLayoutBuilder::clear()
{
	bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shader_stages)
{
	assert(!bindings.empty());

	for (auto& b : bindings)
		b.stageFlags |= shader_stages;

	VkDescriptorSetLayoutCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);

	return layout;
}

void DescriptorAllocator::init(VkDevice device, uint32_t max_sets, const std::span<PoolSizeRatio>& ratios)
{
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(ratios.size());
	for (auto& ratio : ratios)
	{
		sizes.push_back({
			ratio.type,
			static_cast<uint32_t>(ratio.ratio * max_sets)
			});
	}

	VkDescriptorPoolCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = max_sets,
		.poolSizeCount = static_cast<uint32_t>(sizes.size()),
		.pPoolSizes = sizes.data(),
	};

	vkCreateDescriptorPool(device, &info, nullptr, &pool);
}

void DescriptorAllocator::clear(VkDevice device)
{
	vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroy(VkDevice device)
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo info{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout,
	};

	VkDescriptorSet ds;
	VKCHECK(vkAllocateDescriptorSets(device, &info, &ds));

	return ds;
}

void PipelineBuilder::clear()
{
	shader_stages.clear();
	input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f };
	color_blend_attachment = {};
	multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	layout = {};
	depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
}

void PipelineBuilder::set_shaders(VkShaderModule shader_module, VkShaderStageFlags shader_stage)
{
	VkPipelineShaderStageCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = (VkShaderStageFlagBits)shader_stage,
		.module = shader_module,
		.pName = "main",
	};

	shader_stages.push_back(std::move(info));
}

void PipelineBuilder::set_topology(VkPrimitiveTopology topology)
{
	input_assembly.topology = topology;
	input_assembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::set_polygon_mode(VkPolygonMode polygon_mode)
{
	rasterizer.polygonMode = polygon_mode;
}

void PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode_flags, VkFrontFace front_face)
{
	rasterizer.frontFace = front_face;
	rasterizer.cullMode = cull_mode_flags;
}

void PipelineBuilder::set_multisampling_none()
{
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::disable_blending()
{
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::set_color_attachment_format(VkFormat format)
{
	color_attachment_format = format;
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachmentFormats = &color_attachment_format;
}

void PipelineBuilder::set_depth_format(VkFormat format)
{
	render_info.depthAttachmentFormat = format;
}

void PipelineBuilder::disable_depth_test()
{
	depth_stencil.depthTestEnable = VK_FALSE;
	depth_stencil.depthWriteEnable = VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {};
	depth_stencil.back = {};
	depth_stencil.minDepthBounds = 0.0f;
	depth_stencil.maxDepthBounds = 1.0f;
}

VkPipeline PipelineBuilder::build(VkDevice device)
{
	VkVertexInputAttributeDescription pos_attr_desc{
		.location = 0,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(Vertex, position),
	};

	VkVertexInputAttributeDescription uv_attr_desc{
		.location = 1,
		.binding = 0,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(Vertex, uv),
	};

	VkVertexInputAttributeDescription normal_attr_desc{
		.location = 2,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(Vertex, normal),
	};

	VkVertexInputAttributeDescription color_attr_desc{
		.location = 3,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = offsetof(Vertex, color),
	};

	VkVertexInputAttributeDescription attribute_descriptor[4] = { pos_attr_desc, uv_attr_desc, normal_attr_desc, color_attr_desc };

	VkVertexInputBindingDescription input_binding{
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	// TODO Add this through functions.
	VkPipelineVertexInputStateCreateInfo vertex_input_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &input_binding,
		.vertexAttributeDescriptionCount = 4,
		.pVertexAttributeDescriptions = attribute_descriptor,
	};

	VkViewport viewport{
		.x = 0,
		.y = 0,
	};

	VkRect2D scissors{
		.offset = {0, 0},
		.extent = {0, 0},
	};

	VkPipelineViewportStateCreateInfo viewport_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineColorBlendStateCreateInfo color_blend{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_TRUE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
	};

	VkDynamicState state[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = state,
	};

	VkGraphicsPipelineCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(shader_stages.size()),
		.pStages = shader_stages.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depth_stencil,
		.pColorBlendState = &color_blend,
		.pDynamicState = &dynamic_state,
		.layout = layout,
	};

	VkPipeline pipeline;
	VKCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));
	return pipeline;
}
