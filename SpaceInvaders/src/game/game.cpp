#include <engine/camera.h>
#include <engine/timer_manager.h>
#include <engine/resource_manager.h>
#include <game/game.h>
#include <game/entities/enemy.h>
#include <game/entities/meteor.h>
#include <game/entities/player.h>
#include <game/entities/spawner.h>
#include <renderer/render.h>
#include <renderer/render_types.h>

std::map<std::string, Mesh*> meshes;

Game* Game::game = nullptr;

Game::Game()
{
	game = this;
	timer_manager = new TimerManager();
	resource_manager = new ResourceManager();
}

void Game::init(Camera& camera)
{
	init_resources();

	current_camera = &camera;
	current_scene = new Scene();

	auto mesh_plane = resource_manager->get_mesh("plane");

	current_scene->entities.reserve(100);

	Player* player = new Player();
	player->position = glm::vec3(0.0f, 300.0f, 0.0f);
	player->mesh = mesh_plane;
	player->material = resource_manager->get_material("player");

	current_scene->entities.push_back(player);

	Enemy* enemy = new Enemy();
	enemy->position = glm::vec3(0.0f, -300.0f, 0.0f);
	enemy->mesh = mesh_plane;
	enemy->material = resource_manager->get_material("enemy");

	current_scene->entities.push_back(enemy);

	Meteor* meteor = new Meteor();
	meteor->position = glm::vec3(0.0f, -100.0f, 0.0f);
	meteor->mesh = mesh_plane;
	meteor->material = resource_manager->get_material("meteor");

	Meteor* meteor1 = new Meteor();
	meteor1->position = glm::vec3(-100.0f, -100.0f, 0.0f);
	meteor1->mesh = mesh_plane;
	meteor1->material = resource_manager->get_material("meteor");

	Meteor* meteor2 = new Meteor();
	meteor2->position = glm::vec3(100.0f, -100.0f, 0.0f);
	meteor2->mesh = mesh_plane;
	meteor2->material = resource_manager->get_material("meteor");

	Spawner* spawner = new Spawner();

	current_scene->entities.push_back(meteor);
	current_scene->entities.push_back(meteor1);
	current_scene->entities.push_back(meteor2);
	current_scene->entities.push_back(spawner);
}

void Game::shutdown()
{
	for (auto& mesh : meshes)
		destroy_mesh(*mesh.second);

	if (timer_manager)
		delete timer_manager;

	if (resource_manager)
		delete resource_manager;
}

void Game::update(float delta_time)
{
	timer_manager->update(delta_time);

	for (auto& ent : current_scene->entities)
	{
		ent->update(delta_time);
	}
}

void Game::render()
{
	auto render_background = []() {

		};

	auto render_entities = [](const Scene& /*scene*/) {
		};

	auto render_ui = []()
		{

		};

	if (!begin_draw())
		return;

	draw(*current_scene, *current_camera);
	render_background();
	render_entities(*current_scene);
	render_ui();

	end_draw();
}

void Game::init_resources()
{
	Mesh* plane = new Mesh();
	plane->vertices = {
		{{0.5f, -0.5f, 0.0f}, glm::vec2(1.0f, 0.0f), glm::vec3(0.0f), glm::vec4(1.0f)},
		{{0.5f,  0.5f, 0.0f}, glm::vec2(1.0f, 1.0f), glm::vec3(0.0f), glm::vec4(1.0f)},
		{{-0.5f, -0.5f, 0.0f},glm::vec2(0.0f, 0.0f), glm::vec3(0.0f), glm::vec4(1.0f)},
		{{-0.5f, 0.5f, 0.0f}, glm::vec2(0.0f, 1.0f), glm::vec3(0.0f), glm::vec4(1.0f)},
	};

	plane->indices = {
		0, 1, 2, 2, 1, 3
	};

	upload_mesh(*plane);

	resource_manager->create_mesh("plane", std::move(plane));

	uint32_t white = 0xFFFFFFFF;
	resource_manager->create_texture_with_data("white", &white, VkExtent3D{ 1, 1, 1 });
	uint32_t black = 0xFF000000;
	resource_manager->create_texture_with_data("black", &black, VkExtent3D{ 1, 1, 1 });
	uint32_t grey = 0xFFAAAAAA;
	resource_manager->create_texture_with_data("grey", &grey, VkExtent3D{ 1, 1, 1 });
	uint32_t yellow = 0xFF00FFFF;
	resource_manager->create_texture_with_data("yellow", &yellow, VkExtent3D{ 1, 1, 1 });

	resource_manager->load_texture("space_ship", "SpaceInvaders/data/textures/space_ship.png");
	resource_manager->load_texture("enemy_space_ship", "SpaceInvaders/data/textures/enemy_space_ship.png");
	resource_manager->load_texture("meteor", "SpaceInvaders/data/textures/meteor.png");
	resource_manager->load_texture("background", "SpaceInvaders/data/textures/space_background.png");

	{
		Material* m = new Material();
		m->albedo = resource_manager->get_texture("space_ship");
		resource_manager->create_material("player", std::move(m));
	}
	{
		Material* m = new Material();
		m->albedo = resource_manager->get_texture("enemy_space_ship");
		resource_manager->create_material("enemy", std::move(m));
	}
	{
		Material* m = new Material();
		m->albedo = resource_manager->get_texture("meteor");
		resource_manager->create_material("meteor", std::move(m));
	}
	{
		Material* m = new Material();
		m->albedo = resource_manager->get_texture("background");
		resource_manager->create_material("background", std::move(m));
	}
	{
		Material* m = new Material();
		m->albedo = resource_manager->get_texture("yellow");
		resource_manager->create_material("bullet", std::move(m));
	}
}
