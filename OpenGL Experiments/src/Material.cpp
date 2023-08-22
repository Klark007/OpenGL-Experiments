#include "Material.h"

// the same material can be used by multiple meshes
// could add optimization such that multiple materials share the same texture

Material::Material(aiMaterial* material, std::vector<std::pair<aiTextureType, std::string>> types, std::shared_ptr<Program> program, std::string dir)
	: program {program}, directory {dir}
{
	unsigned int size = 0;
	for (std::pair<aiTextureType, std::string> type : types)
		size += material->GetTextureCount(type.first);
	textures = std::vector<Texture>(size);

	unsigned int idx = 0;
	for (std::pair<aiTextureType, std::string> type : types) {
		unsigned int count = material->GetTextureCount(type.first);
		for (unsigned int i = 0; i < count; i++) {
			aiString name;
			material->GetTexture(type.first, i, &name);
			Texture texture;

			texture.ID = load_texutre_from_file(name.C_Str());
			texture.name = type.second + "_" + std::to_string(i);

			textures.at(idx++) = texture;
		}
	}

	set_uniforms();
}

void Material::set_uniforms()
{
	program->use();

	for (int i = 0; i < textures.size(); i++) {
		Texture t = textures.at(i);
		program->set1i(("mat." + t.name).c_str(), i);
	}
}

unsigned int Material::load_texutre_from_file(const char* path)
{
	std::string full_path = directory + '/' + std::string(path);
	return loadTexture(full_path);
}

void Material::use()
{
	program->use();
	
	for (int i = 0; i < textures.size(); i++) {
		Texture t = textures.at(i);

		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, t.ID);
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
