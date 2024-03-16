#pragma once

#include "defines.h"

struct Material;
struct Mesh;
struct Texture;

class ResourceManager
{
public:
	// Only movable.
	ResourceManager() = default;
	~ResourceManager();
	ResourceManager(const ResourceManager& other) = delete;
	ResourceManager(ResourceManager&& other) = default;
	ResourceManager& operator=(const ResourceManager& other) = delete;
	ResourceManager& operator=(ResourceManager&& other) = default;

	void load_mesh(const std::string& name);
	void load_texture(const std::string& name, const std::string& filepath);
	void create_texture_with_data(const std::string& name, void* data, VkExtent3D extent);
	void create_mesh(const std::string& name, Mesh* m);
	void create_material(const std::string& name, Material* m);

	Mesh* get_mesh(const std::string& name) const;
	Texture* get_texture(const std::string& name) const;
	Material* get_material(const std::string& name) const;

private:
	std::unordered_map<std::string, Mesh*> meshes;
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, Material*> materials;
};