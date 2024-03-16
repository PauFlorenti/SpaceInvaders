#pragma once

#include <defines.h>

class Input
{
public:
	using InputIndex = int32_t;

	static bool init(GLFWwindow* window);
	static void update();

	static bool is_key_down(InputIndex key);
	static bool was_key_down(InputIndex key);
	static bool is_button_down(InputIndex button);
	static bool was_button_down(InputIndex button);
	static float get_axis(InputIndex axis);

private:
	static void key_callback(GLFWwindow* window, int32_t key, int32_t /*scancode*/, int action, int /*mods*/);

	static GLFWwindow* window;

	static std::map<InputIndex, bool> keys;
	static std::map<InputIndex, bool> previous_keys;
	static char buttons[GLFW_GAMEPAD_BUTTON_LAST]; // Should differentiate in another class?
	static char previous_buttons[GLFW_GAMEPAD_BUTTON_LAST];
	static float axes[GLFW_GAMEPAD_AXIS_LAST];
};
