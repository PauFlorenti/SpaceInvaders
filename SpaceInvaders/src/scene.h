#pragma once

#include <defines.h>
#include <input.h>

struct Mesh;
struct Material;

extern InputState input_state;

struct Entity
{
	glm::vec3 position = glm::vec3(0.0f);
	Mesh* mesh = nullptr;
	Material* material = nullptr;

	virtual void update(const float /*delta_time*/) {};
};

struct Player : Entity
{
	void update(const float delta_time) override;
};

struct Enemy : Entity
{
	void update(const float /*delta_time*/) override {};
};

struct Meteor : Entity
{
	void update(const float delta_time) override;
};

struct Scene
{
	std::vector<Entity*> entities;
};
