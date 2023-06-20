#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 pos, glm::vec3 des, glm::vec3 up);

	inline void set_pos(glm::vec3 pos) { position = pos; }
	inline void look_at(glm::vec3 des) { destination = des; }
	inline void set_up(glm::vec3 up) { up = glm::normalize(up);  }
private:
	glm::vec3 position;
	glm::vec3 up;
	glm::vec3 destination;

public:
	glm::mat4 generate_view_mat();
	inline glm::vec3 get_pos() { return position; };
};

