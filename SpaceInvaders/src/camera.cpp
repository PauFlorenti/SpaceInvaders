#include "camera.h"

Camera::Camera()
{
	look_at(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	set_orthographic(-100, 100, 100, -100, -100, 100);
}

void Camera::set_orthographic(float new_left, float right, float top, float bottom, float new_near, float new_far)
{
	type = Type::ORTHOGRAPHIC;

	near = new_near;
	far = new_far;

	projection = glm::ortho(new_left, right, top, bottom, near, far);
	view_projection = get_projection() * view;
}

void Camera::look_at(glm::vec3 new_eye, glm::vec3 new_target, glm::vec3 new_up)
{
	eye = new_eye;
	forward = new_target;
	view = glm::lookAt(eye, forward, new_up);
	view_projection = get_projection() * view;

	forward = glm::normalize(forward - eye);
	left = glm::normalize(glm::cross(up, forward));
	up = glm::normalize(glm::cross(forward, left));
}

glm::mat4 Camera::get_view() const
{
	return view;
}

glm::mat4 Camera::get_projection() const
{
	return projection;
}

glm::mat4 Camera::get_view_projection() const
{
	return view * projection;
}
