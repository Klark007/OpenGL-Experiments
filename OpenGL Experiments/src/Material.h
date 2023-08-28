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
	Material(aiMaterial* material, std::vector<std::pair<aiTextureType, std::string>> types, std::string dir);
	Material(std::vector<std::pair<std::string, std::string>> texture_list, std::string dir);
private:
	std::vector<Texture> textures;

	std::string directory;

	void set_uniforms(Program& program);
	unsigned int load_texture_relative(const char* path);
public:
	void use(Program& program);
};

unsigned int loadTexture(std::string path);
