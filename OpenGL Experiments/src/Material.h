#pragma once

#include "Program.h"
#include "stb_image.h"
#include "Texture.h"
#include <assimp/scene.h>

#include <vector>
#include <memory>


class Material
{
public:
	Material(aiMaterial* material, std::vector<std::pair<aiTextureType, std::string>> types, std::string dir);
	Material(std::vector<std::pair<std::string, std::string>> texture_list, std::string dir);
private:
	std::vector<TextureP> textures;

	std::string directory;

	void set_uniforms(Program& program);
	unsigned int load_texture_relative(const char* path);

	TextureP create_texture(std::string path, std::string name);
public:
	void use(Program& program);
};

unsigned int loadTexture(std::string path);
