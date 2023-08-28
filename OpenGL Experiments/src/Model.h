#pragma once

#include "Material.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <memory>

// Model assumes one shader program per object

class Model
{
public:
	Model(std::string path);
	void draw(Program& program);

private:
	std::shared_ptr<Program> program;

	std::vector<Mesh> meshes;
	std::vector<std::shared_ptr<Material>> materials;

	std::string directory;

	void load_node(aiNode* node, const aiScene* scene);
};

