#include "Program.h"

Program::Program()
{
	program = glCreateProgram();
}

Program::Program(std::vector<std::shared_ptr<Shader>>& shaders)
{
	program = glCreateProgram();

	for (std::shared_ptr<Shader> s : shaders) {
		attach_shader(s->get_id());
	}

	link_program();
	print_link_error();
}

Program::~Program()
{
	glDeleteProgram(program);
}

void Program::attach_shader(unsigned int id)
{
	glAttachShader(program, id);
}

void Program::attach_shader(Shader& shader)
{
	glAttachShader(program, shader.get_id());
}

void Program::link_program()
{
	glLinkProgram(program);
}

void Program::use()
{
	glUseProgram(program);
}

void Program::get(unsigned int name, int* params)
{
	glGetProgramiv(program, name, params);
}

void Program::print_link_error()
{
	int status;
	get(GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		glGetProgramInfoLog(program, LOG_LENGTH, NULL, log);
		std::cerr << "PROGRAMM LINKING FAILED:" << log << std::endl;
		std::cerr << status << std::endl;
	}
}

void Program::set1i(const char* name, int value)
{
	glUniform1i(get_location(name), value);
}

void Program::set1f(const char* name, float value)
{
	glUniform1f(get_location(name), value);
}

void Program::set_vec2f(const char* name, glm::vec2& value)
{
	glUniform2fv(get_location(name), 1, glm::value_ptr(value));
}

void Program::set_vec2f(const char* name, float x, float y)
{
	glUniform2f(get_location(name), x, y);
}

void Program::set_vec3f(const char* name, glm::vec3& value)
{
	glUniform3fv(get_location(name), 1, glm::value_ptr(value));
}

void Program::set_vec3f(const char* name, float r, float g, float b)
{
	glUniform3f(get_location(name), r, g, b);
}

void Program::set_mat4f(const char* name, glm::mat4& value)
{
	glUniformMatrix4fv(get_location(name), 1, GL_FALSE, glm::value_ptr(value));
}
