#include <game/entities/meteor.h>

Meteor::Meteor()
{
	scale = glm::vec3(50.0f);
}

void Meteor::update(const float delta_time)
{
	position += direction * delta_time * 500.0f;
}