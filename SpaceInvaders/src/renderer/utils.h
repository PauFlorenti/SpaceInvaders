#pragma once

struct VulkanState;

void immediate_submit(VulkanState* state, std::function<void(VkCommandBuffer cmd)>&& function);

bool load_shader_module(const char* filename, VkDevice device, VkShaderModule* out_shader_module);

void upload_mesh_utils(VulkanState* state, Mesh& mesh);

void destroy_mesh_utils(VulkanState* state, Mesh& mesh);