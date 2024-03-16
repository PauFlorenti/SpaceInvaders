#pragma once

#include "defines.h"
#include <game/entities/entity.h>

struct Meteor;

struct Spawner : Entity
{
	float spawn_rate = 2000.0f; // Every 2 seconds.
	float current_rate = 0.0f;

	Spawner();
	Meteor* spawn_meteor();
};