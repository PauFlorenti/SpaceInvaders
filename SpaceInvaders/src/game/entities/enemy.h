#pragma once

#include "defines.h"
#include <game/entities/entity.h>

struct Enemy : Entity
{
	enum State
	{
		MOVE = 0,
		FIRE,
		COUNT
	};

	Enemy();
	void update(const float delta_time) override;
	void destroy() override;
	void fire();

	State current_state = State::MOVE;
	glm::vec3 target = glm::vec3(0.0f);
};