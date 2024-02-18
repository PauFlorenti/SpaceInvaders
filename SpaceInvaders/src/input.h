#pragma once

#include <defines.h>

struct InputState
{
	std::map<int32_t, bool> keys;
	GLFWwindow* window = nullptr;
};

bool init_input(InputState& input_state, GLFWwindow* window);

bool is_key_down(InputState& input_state, int key);
