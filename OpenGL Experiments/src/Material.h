#pragma once

#include "Program.h"
#include "stb_image.h"
#include <assimp/scene.h>

#include <vector>
#include <memory>


struct Texture {
	unsigned int ID;
	std::string name;
};

class Material
{
public:
	Material(aiMaterial* material, std::vector<std::pair<aiTextureType, std::string>> types, std::shared_ptr<Program> program, std::string dir);
private:
	std::vector<Texture> textures;
	std::shared_ptr<Program> program;

	std::string directory;

	void set_uniforms();
	unsigned int load_texutre_from_file(const char* path);
public:
	void use();
};

unsigned int loadTexture(std::string path);
