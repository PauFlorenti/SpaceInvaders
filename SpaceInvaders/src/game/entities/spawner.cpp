#include <engine/resource_manager.h>
#include <engine/timer_manager.h>
#include <game/entities/spawner.h>
#include <game/entities/meteor.h>
#include <game/game.h>

Spawner::Spawner()
{
	auto& timer_manager = Game::Get()->get_timer_manager();

	TimerHandle timer_handle;
	timer_manager.set_timer(timer_handle, 3.0f, []() {
		auto game = Game::Get();
		const auto resource_manager = game->get_resource_manager();
		assert(game);
		assert(game->current_scene);

		Meteor* meteor = new Meteor();

		auto random = (rand() % 1000) - 500;
		meteor->position = glm::vec3((float)random, -500.0f, 0.0f);
		meteor->mesh = resource_manager->get_mesh("Plane");
		meteor->material = resource_manager->get_material("meteor");
		meteor->scale = glm::vec3(50.0f);

		game->current_scene->entities.push_back(meteor);
		}, true);
}

Meteor* Spawner::spawn_meteor()
{
	const auto game = Game::Get();
	const auto resource_manager = game->get_resource_manager();
	assert(game);
	assert(game->current_scene);

	auto meteor = new Meteor();
	auto random = (rand() % 1000) - 500;
	meteor->position = glm::vec3((float)random, -500.0f, 0.0f);
	meteor->mesh = resource_manager->get_mesh("Plane");
	meteor->material = resource_manager->get_material("meteor");
	meteor->scale = glm::vec3(50.0f);

	game->current_scene->entities.push_back(meteor);

	return meteor;
}
