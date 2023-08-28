#pragma once

#include "Material.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texture_coord;
};

typedef unsigned int index;

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<index> indices, std::shared_ptr<Material> material);
	void draw(Program& program);

private:
	std::vector<Vertex> vertices;
	std::vector<index> indices;
	std::shared_ptr<Material> material;

	unsigned int vao, vbo, ebo;

	void setup_mesh();
};