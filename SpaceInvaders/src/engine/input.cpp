#include <engine/input.h>

GLFWwindow* Input::window = nullptr;
std::map<Input::InputIndex, bool> Input::keys;
std::map<Input::InputIndex, bool> Input::previous_keys;
char Input::buttons[GLFW_GAMEPAD_BUTTON_LAST] = {}; // Should differentiate in another class?
char Input::previous_buttons[GLFW_GAMEPAD_BUTTON_LAST] = {};
float Input::axes[GLFW_GAMEPAD_AXIS_LAST] = {};

void Input::key_callback(GLFWwindow* /*window*/, int32_t key, int32_t /*scancode*/, int action, int /*mods*/)
{
	if (action == GLFW_PRESS)
	{
		previous_keys.at(key) = keys.at(key);
		keys.at(key) = true;
	}
	else if (action == GLFW_RELEASE)
	{
		previous_keys.at(key) = keys.at(key);
		keys.at(key) = false;
	}
}

// TODO Should we have initialize and deinitialize gamepad logic??
static void joystick_callback(int32_t joystick, int32_t event)
{
	printf("Joystick callback called. Currently doing nothing.\n");

	if (event == GLFW_CONNECTED)
	{
		const auto gamepad_name = glfwGetGamepadName(joystick);
		printf("[INPUT] - Joystick %i connected.\n", joystick);
		printf("[INPUT] - Name: '%s'.\n", gamepad_name);
	}
	else if (event == GLFW_DISCONNECTED)
	{
		printf("[INPUT] - Joystick disconnected.\n");
	}
}

bool Input::init(GLFWwindow* in_window)
{
	window = in_window;
	assert(window != nullptr);

	// Set all keys to false;
	for (int key = 0; key < GLFW_KEY_LAST; ++key)
	{
		keys[key] = false;
		previous_keys[key] = false;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetJoystickCallback(joystick_callback);

	if (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
	{
		const auto gamepad_name = glfwGetGamepadName(GLFW_JOYSTICK_1);
		printf("[INPUT] - Gamepad %s found!\n", gamepad_name);
	}

	return true;
}

void Input::update()
{
	assert(keys.size() == previous_keys.size());
	previous_keys = keys;

	if (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
	{
		GLFWgamepadstate gamepad;
		auto ok = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad);
		assert(ok);

		memcpy(axes, gamepad.axes, sizeof(float) * GLFW_GAMEPAD_AXIS_LAST);
		memcpy(previous_buttons, buttons, sizeof(char) * GLFW_GAMEPAD_BUTTON_LAST);
		memcpy(buttons, gamepad.buttons, sizeof(char) * GLFW_GAMEPAD_BUTTON_LAST);
	}
}

bool Input::is_key_down(InputIndex key)
{
	return keys.at(key);
}

bool Input::was_key_down(InputIndex key)
{
	return (previous_keys.at(key) == true) && (keys.at(key) == false);
}

bool Input::is_button_down(InputIndex button)
{
	return buttons[button];
}

bool Input::was_button_down(InputIndex button)
{
	return (previous_buttons[button] && !buttons[button]);
}

float Input::get_axis(InputIndex axis)
{
	return axes[axis];
}
