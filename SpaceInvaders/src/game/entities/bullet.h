#pragma once

#include "defines.h"
#include <game/entities/entity.h>

struct Bullet : Entity
{
	Bullet();
	void update(const float delta_time) override;
	void destroy() override;
	uint32_t damage = 10;
};