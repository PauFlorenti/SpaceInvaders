#include <engine/resource_manager.h>

#include <renderer/render.h> 
#include <renderer/render_types.h>

ResourceManager::~ResourceManager()
{
	for (auto& t : textures)
	{
		destroy_texture(*t.second);
	}

	for (auto& m : meshes)
	{
		destroy_mesh(*m.second);
	}
}

void ResourceManager::load_mesh(const std::string& /*name*/)
{
	// TODO load meshes.
}

void ResourceManager::load_texture(const std::string& name, const std::string& filename)
{
	if (textures.count(name) == 0)
	{
		textures.insert({ name, std::move(create_texture(filename)) });
	}
}

void ResourceManager::create_texture_with_data(const std::string& name, void* data, VkExtent3D extent)
{
	assert(data != nullptr);

	if (textures.count(name) == 0)
	{
		textures.insert({ name, create_texture(data, extent) });
	}
}

void ResourceManager::create_mesh(const std::string& name, Mesh* m)
{
	assert(m);

	if (meshes.count(name) == 0)
	{
		meshes.insert({ name, m });
	}
	else
	{
		printf("[WARN]: Mesh with name %s has already been found in the resource_manager.", name.c_str());
	}
}

void ResourceManager::create_material(const std::string& name, Material* m)
{
	assert(m);

	if (materials.count(name) == 0)
	{
		materials.insert({name, m});
	}
	else
	{
		printf("[WARN]: Material with name %s has already been found in the resource_manager.", name.c_str());
	}
}

Mesh* ResourceManager::get_mesh(const std::string& name) const
{
	if (meshes.count(name) > 0)
	{
		return meshes.find(name)->second;
	}

	printf("[WARN]: Texture %s not found in resource manager.", name.c_str());
	return nullptr;
}

Texture* ResourceManager::get_texture(const std::string& name) const
{
	if (textures.count(name) > 0)
	{
		return textures.find(name)->second;
	}

	printf("[WARN]: Texture %s not found in resource manager.", name.c_str());
	return nullptr;
}

Material* ResourceManager::get_material(const std::string& name) const
{
	if (materials.count(name) > 0)
	{
		return materials.find(name)->second;
	}

	printf("[WARN]: Material %s not found in resource manager.", name.c_str());
	return nullptr;
}
