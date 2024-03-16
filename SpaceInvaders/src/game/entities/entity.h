#pragma once
#include <defines.h>

struct Mesh;
struct Material;

struct Entity
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 scale = glm::vec3(0.0f);
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	int32_t health = 1;
	float speed = 250.0f;

	virtual void update(const float /*delta_time*/) {};
	virtual void destroy() {};
};
