#include <engine/input.h>
#include <game/entities/player.h>
#include <game/game.h>
#include <game/entities/bullet.h>

Player::Player()
{
	scale = glm::vec3(50.0f);
}

void Player::update(const float delta_time)
{
	if (Input::is_key_down(GLFW_KEY_A) || Input::is_button_down(GLFW_GAMEPAD_BUTTON_DPAD_LEFT) || Input::get_axis(GLFW_GAMEPAD_AXIS_LEFT_X) < -0.3f)
	{
		position.x += delta_time * 0.1f * speed;
	}

	if (Input::is_key_down(GLFW_KEY_D) || Input::is_button_down(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT) || Input::get_axis(GLFW_GAMEPAD_AXIS_LEFT_X) > 0.3f)
	{
		position.x -= delta_time * 0.1f * speed;
	}

	if (Input::is_key_down(GLFW_KEY_W) || Input::is_button_down(GLFW_GAMEPAD_BUTTON_DPAD_UP) || Input::get_axis(GLFW_GAMEPAD_AXIS_LEFT_Y) < -0.3f)
	{
		position.y -= delta_time * 0.1f * speed;
	}

	if (Input::is_key_down(GLFW_KEY_S) || Input::is_button_down(GLFW_GAMEPAD_BUTTON_DPAD_DOWN) || Input::get_axis(GLFW_GAMEPAD_AXIS_LEFT_Y) > 0.3f)
	{
		position.y += delta_time * 0.1f * speed;
	}

	if (Input::was_button_down(GLFW_GAMEPAD_BUTTON_A) || Input::was_button_down(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER) || Input::was_key_down(GLFW_KEY_SPACE))
	{
		fire();
	}
}

void Player::destroy()
{
	// TODO When scenes and level, open GAME OVER menu.
	printf("Player destroyed!");
}

void Player::fire()
{
	const auto game = Game::Get();
	assert(game);
	assert(game->current_scene);

	auto b = new Bullet();
	b->position = position + glm::vec3(0.0f, -30.0f, 0.0f);
	b->direction = glm::vec3(0.0f, -1.0f, 0.0f);

	game->current_scene->entities.push_back(std::move(b));
}