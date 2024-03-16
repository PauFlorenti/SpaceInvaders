#include <defines.h>

#include <engine/camera.h>
#include <engine/game.h>
#include <engine/input.h>
#include <renderer/render.h>

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

	if (!Input::init(window))
	{
		printf("Failed to init input system.");
		return -1;
	}

	int32_t w, h;
	glfwGetWindowSize(window, &w, &h);

	Camera camera;
	camera.set_orthographic(-((float)w / 2.0f), ((float)w / 2.0f), (float)h / 2.0f, -((float)h / 2.0f), -100.0f, 100.0f);

	Game game;
	game.init(camera);

	static auto start_time = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point last_time = start_time;

	while (!glfwWindowShouldClose(window))
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_since_start = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time);
		const auto elapsed_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		Input::update();
		glfwPollEvents();

		game.update(elapsed_time.count());
		game.render();

		last_time = current_time;
	}

	render_wait_idle();

	game.shutdown();

	render_shutdown();

	glfwTerminate();
	glfwDestroyWindow(window);

	return 0;
}