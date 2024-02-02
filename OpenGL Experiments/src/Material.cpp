#include "Material.h"

#include <unordered_map>

// the same material can be used by multiple meshes
// could add optimization such that multiple materials share the same texture

Material::Material(aiMaterial* material, std::vector<std::pair<aiTextureType, std::string>> types, std::string dir)
	: directory {dir}
{
	textures = std::vector<TextureP>();

	unsigned int idx = 0;
	for (std::pair<aiTextureType, std::string> type : types) {
		unsigned int count = material->GetTextureCount(type.first);
		for (unsigned int i = 0; i < count; i++) {
			aiString name;
			material->GetTexture(type.first, i, &name);

			load_texture_relative(name.C_Str());
			textures.push_back(create_texture(name.C_Str(), type.second + "_" + std::to_string(i)));
		}
	}
}

Material::Material(std::vector<std::pair<std::string, std::string>> texture_list, std::string dir)
	: directory{ dir }
{
	textures = std::vector<TextureP>();
	std::unordered_map<std::string, unsigned int> counters; // counts the number of already existing textures for each type (diffuse, specular, etc.)

	for (unsigned int i = 0; i < texture_list.size(); i++) {
		std::pair<std::string, std::string> t = texture_list.at(i);
		
		unsigned int c;
		if (counters.find(t.first) == counters.end()) {
			counters[t.first] = 0;
			c = 0;
		}
		else {
			c = ++counters[t.first];
		}
		load_texture_relative(t.second.c_str());
		textures.push_back(create_texture(t.second, t.first + "_" + std::to_string(c)));
	}
}

void Material::set_uniforms(Program& program)
{
	program.use();

	for (int i = 0; i < textures.size(); i++) {
		textures.at(i)->set_texture_unit(program,i);
	}
}

unsigned int Material::load_texture_relative(const char* path)
{
	std::string full_path = directory + '/' + std::string(path);
	return loadTexture(full_path);
}

TextureP Material::create_texture(std::string path, std::string name)
{
	std::string full_path = directory + '/' + path;

	int width, height, channels;
	unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &channels, 0);

	if (data) {
		// should choose internal format, potential issues with format not being one of the defined interal formats
		GLenum format;
		if (channels == 1)
			format = GL_RED;
		if (channels == 2)
			format = GL_RG;
		if (channels == 3)
			format = GL_RGB;
		if (channels == 4)
			format = GL_RGBA;

		std::string n = std::string("mat.")+name;
		TextureP t = std::make_unique<Texture>(
			n,
			width,
			height,
			data,
			format,
			format,
			Texture_Filter::linear,
			Texture_Wrap::repeat,
			false
		);

		stbi_image_free(data);
		return t;
	}
	else
	{
		std::cerr << "Failed to load texture at: " << path << std::endl;
		stbi_image_free(data);
		throw std::runtime_error("No texture found");
	}
}

void Material::use(Program& program)
{
	set_uniforms(program);
	
	for (int i = 0; i < textures.size(); i++) {
		textures.at(i)->bind();
	}
	glActiveTexture(GL_TEXTURE0);
}

unsigned int loadTexture(std::string path) {
	unsigned int id;
	glGenTextures(1, &id);

	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	if (data) {
		// should choose internal format, potential issues with format not being one of the defined interal formats
		GLenum format;
		if (channels == 1)
			format = GL_RED;
		if (channels == 2)
			format = GL_RG;
		if (channels == 3)
			format = GL_RGB;
		if (channels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Failed to load texture at: " << path << std::endl;
		stbi_image_free(data);
	}

	return id;
}
