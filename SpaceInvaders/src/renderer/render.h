#pragma once

#include <defines.h>

class Camera;
struct Mesh;
struct Scene;
struct Texture;

bool render_init(GLFWwindow* window);
bool begin_draw();
void draw(const Scene& scene, const Camera& camera);
void end_draw();
void upload_mesh(Mesh& mesh);
void destroy_mesh(Mesh& mesh);
Texture* create_texture(std::string texture_filename);
void destroy_texture(Texture& texture);
void render_wait_idle();
void render_shutdown();