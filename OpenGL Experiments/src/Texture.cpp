#include "Texture.h"

Texture_Filter get_mipmap_filter(Texture_Filter f) {
	switch (f)
	{
	case Texture_Filter::linear:
		return Texture_Filter::linear_mipmap;
	case Texture_Filter::nearest:
		return Texture_Filter::nearest_mipmap;
	}
}

Texture::Texture(std::string& n, unsigned int res_x, unsigned int res_y, float* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap, bool mip_map)
{
	name = n;
	type = GL_TEXTURE_2D;

	glGenTextures(1, &id);
	glBindTexture(type, id);

	glTexImage2D(type, 0, internal_format, res_x, res_y, 0, format, GL_FLOAT, data);

	if (mip_map) {
		glGenerateMipmap(type);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, get_mipmap_filter(filter));
	}
	else {
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filter);
	}
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap);
}

Texture::Texture(std::string& n, unsigned int res_x, unsigned int res_y, unsigned char* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap, bool mip_map)
{
	name = n;
	type = GL_TEXTURE_2D;

	glGenTextures(1, &id);
	glBindTexture(type, id);

	glTexImage2D(type, 0, internal_format, res_x, res_y, 0, format, GL_UNSIGNED_BYTE, data);
	
	if (mip_map) {
		glGenerateMipmap(type);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, get_mipmap_filter(filter));
	}
	else {
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filter);
	}
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap);
}

Texture::Texture(std::string& n, unsigned int res_x, unsigned int res_y, unsigned int res_z, float* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap)
{
}

Texture::Texture(std::string& n, unsigned int res_x, unsigned int res_y, unsigned int res_z, unsigned char* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap)
{
}

Texture::~Texture()
{
	std::cout << "DELETE" << std::endl;
	glDeleteTextures(1,&id);
}

void Texture::set_texture_unit(Program& program, int unit)
{
	texture_unit = unit;
	program.set1i(name.c_str(), texture_unit);
}

void Texture::bind()
{
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(type,id);
}

void Texture::bind_texture_unit(Program& program, int unit)
{
	set_texture_unit(program,unit);
	bind();
}
