#include "Program.h"

Program::Program()
{
	program = glCreateProgram();
}

Program::~Program()
{
	glDeleteProgram(program);
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
	}
}

void Program::set1f(const char* name, float value)
{
	glUniform1f(get_location(name), value);
}
