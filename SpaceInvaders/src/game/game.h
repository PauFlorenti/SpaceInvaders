#pragma once

// TODO This should not be included.
// Should go into a game/ folder.
#include <engine/scene.h>

class Camera;
class ResourceManager;
class TimerManager;

class Game
{
public:
	static Game* Get()
	{
		if (game)
			return game;
		return new Game();
	}

	Game();

	void init(Camera& camera);
	void shutdown();
	void update(float delta_time);
	void render();

	TimerManager& get_timer_manager() const { return *timer_manager; }
	const ResourceManager* get_resource_manager() const { return resource_manager; }

	Scene* current_scene = nullptr;
private:
	void init_resources();

	static Game* game;
	Camera* current_camera = nullptr;

	// TODO Should be handled by engine, not by game.
	TimerManager* timer_manager = nullptr;
	ResourceManager* resource_manager = nullptr;
};