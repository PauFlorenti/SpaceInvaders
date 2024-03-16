#include <engine/scene.h>
#include <engine/resource_manager.h>
#include <renderer/render_types.h>
#include <game/entities/entity.h>
#include <game/game.h>

Scene::Scene()
{
	const auto resource_manager = Game::Get()->get_resource_manager();
	background = new Entity();
	background->mesh = resource_manager->get_mesh("Plane");
	background->material = resource_manager->get_material("background");
}
