#include "Shader.h"

Shader::Shader(unsigned int shader_type)
{
	shader = glCreateShader(shader_type);
}

Shader::~Shader()
{
	glDeleteShader(shader);
}

int Shader::add_source_from_file(const char* path)
{
	std::ifstream file;
	file.open(path);

	if (!file.is_open()) {
		std::cerr << "SHADER FILE NOT FOUND AT:" << path << std::endl;
		return -1;
	}
		
	std::stringstream source_stream;
	source_stream << file.rdbuf();

	std::string source_string = source_stream.str();
	const char* source_code = source_string.c_str();

	glShaderSource(shader, 1, &source_code, NULL);
	return 0;
}

void Shader::compile()
{
	glCompileShader(shader);
}

void Shader::get(unsigned int name, int* params)
{
	glGetShaderiv(shader, name, params);
}

void Shader::print_compile_error()
{
	int status;
	get(GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		glGetShaderInfoLog(shader, LOG_LENGTH, NULL, log);
		std::cerr << "SHADER COMPILATION FAILED:" << log << std::endl;
	}
}