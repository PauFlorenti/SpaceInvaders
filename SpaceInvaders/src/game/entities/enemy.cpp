#include <game/entities/bullet.h>
#include <game/entities/enemy.h>
#include <game/game.h>
#include <engine/timer_manager.h>

Enemy::Enemy()
{
	direction = glm::vec3(1.0f, 0.0f, 0.0f);
	scale = glm::vec3(50.0f);
	target = glm::vec3(500.0f, position.y, position.z);
	//target = glm::vec3(static_cast<float>(rand() % 500), this->position.y, this->position.z);
	direction = glm::normalize(target - position);

	auto& timer_manager = Game::Get()->get_timer_manager();

	auto callback = [&]() {
		printf("Callback!\n");
		};

	TimerHandle timer_handle;
	timer_manager.set_timer(timer_handle, 1.0f, callback, true);

}

void Enemy::update(const float delta_time)
{
	auto calculate_next_point = [this]()
		{
			const auto random = static_cast<float>(rand() % 500);
			this->target = this->position;
			if (this->position.x > 0.0f)
			{
				this->target.x = -random;
			}
			else
			{
				this->target.x = random;
			}
		};

	auto distance_to_target = [this]()
		{
			return abs(this->target.x - this->position.x);
		};

	switch (current_state)
	{
	case Enemy::MOVE:

		if (distance_to_target() > 100.0f)
		{
			position += direction * delta_time * 10.0f;
			break;
		}

		direction = glm::normalize(target - position);
		calculate_next_point();
		current_state = State::FIRE;

		break;
	case Enemy::FIRE:
		fire();
		current_state = State::MOVE;
		break;
	case Enemy::COUNT:
	default:
		assert(false);
		break;
	}
}

void Enemy::destroy()
{
}

void Enemy::fire()
{
	printf("Pinyau!\n");
	const auto game = Game::Get();
	assert(game);
	assert(game->current_scene);

	auto b = new Bullet();
	b->position = position + glm::vec3(0.0f, -10.0f, 0.0f);
	b->direction = glm::vec3(0.0f, -1.0f, 0.0f);

	game->current_scene->entities.push_back(std::move(b));
}