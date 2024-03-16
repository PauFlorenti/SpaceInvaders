#include <engine/resource_manager.h>
#include <game/entities/bullet.h>
#include <game/game.h>

Bullet::Bullet()
{
	auto resource_manager = Game::Get()->get_resource_manager();
	mesh = resource_manager->get_mesh("Plane");
	material = resource_manager->get_material("bullet");
	scale = glm::vec3(2.5f);
	speed = 100.0f;
}

void Bullet::update(const float delta_time)
{
	position += direction * speed * delta_time;
}

void Bullet::destroy()
{
}
