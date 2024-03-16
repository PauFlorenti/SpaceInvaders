#pragma once

#include <defines.h>

class Camera;
struct Mesh;
struct Scene;
struct Texture;

bool render_init(GLFWwindow* window);
bool begin_draw();
void draw(Scene& scene, const Camera& camera);
void end_draw();
void upload_mesh(Mesh& mesh);
void destroy_mesh(Mesh& mesh);
Texture* create_texture(const std::string texture_filename);
Texture* create_texture(void* data, VkExtent3D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT);
void destroy_texture(Texture& texture);
void render_wait_idle();
void render_shutdown();