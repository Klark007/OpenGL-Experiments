#include "Camera.h"

Camera::Camera()
{
	position = glm::vec3(0.0, 2.0, 10.0);
	up = glm::vec3(0.0, 1.0, 0.0);
	destination = glm::vec3(0.0, 0.0, 0.0);
}

Camera::Camera(glm::vec3 pos, glm::vec3 des, glm::vec3 up)
	: position{ pos }, destination{ des }, up{ up } {}

glm::mat4 Camera::generate_view_mat()
{
	return glm::lookAt(position, destination, up);
}
