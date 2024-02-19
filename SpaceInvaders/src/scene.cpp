#include <scene.h>

InputState input_state;

void Player::update(const float delta_time)
{
	if (is_key_down(input_state, GLFW_KEY_A))
	{
		position.x += delta_time * 0.1f;
	}

	if (is_key_down(input_state, GLFW_KEY_D))
	{
		position.x -= delta_time * 0.1f;
	}

	if (is_key_down(input_state, GLFW_KEY_W))
	{
		position.y -= delta_time * 0.1f;
	}

	if (is_key_down(input_state, GLFW_KEY_S))
	{
		position.y += delta_time * 0.1f;
	}
}

void Meteor::update(const float delta_time)
{
	position.y += delta_time * 0.4f;
}
