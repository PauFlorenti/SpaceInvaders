#pragma once

#include <defines.h>
#include <input.h>

struct Mesh;
struct Texture;

extern InputState input_state;

struct Entity
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::mat4 transform;
	Mesh* mesh = nullptr;
	Texture* texture = nullptr;

	virtual void update(const float /*delta_time*/)
	{
		//position.x += delta_time;
		printf("Entity in transform!\n");
	}

	virtual void draw() const
	{

	}
};

struct Player : Entity
{
	void update(const float delta_time) override;
};

struct Scene
{
	std::vector<Entity> entities;
	Player player;
};
