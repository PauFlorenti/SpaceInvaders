#pragma once

#include "defines.h"
#include <game/entities/entity.h>

struct Meteor : Entity
{
	Meteor();
	void update(const float delta_time) override;
};
