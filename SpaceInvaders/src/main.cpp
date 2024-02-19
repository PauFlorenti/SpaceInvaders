#include <defines.h>

#include <camera.h>
#include <input.h>
#include <renderer/render.h>
#include <renderer/render_types.h>
#include <scene.h>

std::map<std::string, Mesh*> meshes;
std::map<std::string, Texture*> textures;

static void init_game(Scene& scene)
{
	auto mesh_plane = meshes.find("Plane")->second;
	scene.entities.reserve(100);
	scene.player.mesh = mesh_plane;
	scene.player.texture = textures.find("space_ship")->second;

	Entity enemy;
	enemy.position = glm::vec3(0.0f, 50.0f, 0.0f);
	enemy.mesh = mesh_plane;
	enemy.texture = textures.find("enemy_space_ship")->second;

	scene.entities.push_back(enemy);
}

static void render_game(Scene& scene, Camera& camera)
{
	auto render_background = []() {

		};

	auto render_entities = [](const Scene& scene) {
		for (const auto& ent : scene.entities)
		{
			ent.draw();
		}
		};

	auto render_ui = []()
		{

		};

	if (!begin_draw())
		return;

	draw(scene, camera);
	render_background();
	render_entities(scene);
	render_ui();

	end_draw();
}

static void update_game(Scene& scene, const float delta_time)
{
	if (is_key_down(input_state, GLFW_KEY_F))
	{
		printf("Key F pressed!");
	}

	scene.player.update(delta_time);

	for (auto& ent : scene.entities)
	{
		ent.update(delta_time);
	}
}

static void init_resources()
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

	meshes["Plane"] = std::move(plane);

	textures["space_ship"] = create_texture("SpaceInvaders/data/textures/space_ship.png");
	textures["enemy_space_ship"] = create_texture("SpaceInvaders/data/textures/enemy_space_ship.png");
}

int main()
{
	GLFWwindow* window = nullptr;

	if (!glfwInit())
	{
		printf("Failed to init glfw.");
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(1280, 720, "SpaceInvaders", nullptr, nullptr);

	if (!window)
	{
		printf("Failed to create window.");
		return -1;
	}

	if (!render_init(window))
	{
		printf("Failed to init renderer.");
		return -1;
	}

	if (!init_input(input_state, window))
	{
		printf("Failed to init input system.");
		return -1;
	}

	init_resources();

	int32_t w, h;
	glfwGetWindowSize(window, &w, &h);

	Camera camera;
	camera.set_orthographic(-((float)w / 2.0f), ((float)w / 2.0f), (float)h / 2.0f, -((float)h / 2.0f), -100.0f, 100.0f);

	Scene game;
	init_game(game);

	static auto start_time = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point last_time = start_time;

	while (!glfwWindowShouldClose(window))
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_since_start = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time);
		const auto elapsed_time = std::chrono::duration<float, std::chrono::milliseconds::period>(current_time - last_time);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		glfwPollEvents();

		render_game(game, camera);
		update_game(game, elapsed_time.count());

		last_time = current_time;
	}

	render_wait_idle();

	for (auto& mesh : meshes)
		destroy_mesh(*mesh.second);

	for (auto& t : textures)
		destroy_texture(*t.second);

	render_shutdown();

	glfwTerminate();
	glfwDestroyWindow(window);

	return 0;
}