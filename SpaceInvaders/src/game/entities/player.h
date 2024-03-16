#pragma once

#include <game/entities/entity.h>

struct Player : Entity
{
	Player();
	void update(const float delta_time) override;
	void destroy() override;
	void fire();
};