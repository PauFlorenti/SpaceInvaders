#include <input.h>

static void key_callback(GLFWwindow* window, int32_t key, int32_t /*scancode*/, int action, int /*mods*/)
{
	if (InputState* state = static_cast<InputState*>(glfwGetWindowUserPointer(window)))
	{
		if (action == GLFW_PRESS)
			state->keys.at(key) = true;
		else if (action == GLFW_RELEASE)
			state->keys.at(key) = false;
	}
}

bool init_input(InputState& input_state, GLFWwindow* window)
{
	assert(window != nullptr);

	// Set all keys to false;
	for (int key = 0; key < GLFW_KEY_LAST; ++key)
		input_state.keys[key] = false;

	glfwSetWindowUserPointer(window, &input_state);
	glfwSetKeyCallback(window, key_callback);

	return true;
}

bool is_key_down(InputState& input_state, int key)
{
	return input_state.keys.at(key);
}
