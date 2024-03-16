#pragma once

#include "defines.h"

struct Entity;

struct Scene
{
	Scene();

	Entity* background;
	std::vector<Entity*> entities;
};
