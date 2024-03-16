#pragma once

#include <defines.h>

class Camera
{
	enum class Type { ORTHOGRAPHIC = 0, PERSPECTIVE };

public:
	Camera();

	void set_orthographic(float left, float right, float top, float bottom, float near, float far);

	void look_at(glm::vec3 eye, glm::vec3 target, glm::vec3 up);
	glm::mat4 get_view() const;
	glm::mat4 get_projection() const;
	glm::mat4 get_view_projection() const;

private:

	glm::vec3 eye;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 left;

	Type type = Type::ORTHOGRAPHIC;

	// Orthographic variables
	float fov;
	float aspect;
	float near;
	float far;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 view_projection;
};