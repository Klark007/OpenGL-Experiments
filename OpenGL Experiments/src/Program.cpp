#include "Program.h"

Program::Program()
{
	program = glCreateProgram();
	
	shaders  = std::unordered_set<std::shared_ptr<Shader>>();
	uniforms = std::unordered_map<std::string, Uniform>();
}

Program::Program(std::vector<std::shared_ptr<Shader>>& shaders)
{
	program = glCreateProgram();

	this->shaders	 = std::unordered_set<std::shared_ptr<Shader>>();
	uniforms			 = std::unordered_map<std::string, Uniform>();

	for (std::shared_ptr<Shader> s : shaders) {
		attach_shader(s);
	}

	link_program();
	print_link_error();
}

Program::~Program()
{
	glDeleteProgram(program);
}

void Program::attach_shader(std::shared_ptr<Shader> shader)
{
	shaders.insert(shader);
	glAttachShader(program, shader->get_id());
}

void Program::link_program()
{
	glLinkProgram(program);
}

void Program::recompile()
{
	for (const std::shared_ptr<Shader>& shader : shaders) {
		if (shader->recompile() == -1) {
			return;
		}
	}

	link_program();
	print_link_error();

	// restore uniforms
	for (const auto& uniform : uniforms) {
		uniform.second.set(*this, uniform.first);
	}
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

void Program::set1i(const std::string& name, int value)
{
	uniforms.insert_or_assign(name, Uniform(value));
	glUniform1i(get_location(name), value);
}

void Program::set1f(const std::string& name, float value)
{
	uniforms.insert_or_assign(name, Uniform(value));
	glUniform1f(get_location(name), value);
}

void Program::set_vec2f(const std::string& name, const glm::vec2& value)
{
	uniforms.insert_or_assign(name, Uniform(value));
	glUniform2fv(get_location(name), 1, glm::value_ptr(value));
}

void Program::set_vec2f(const std::string& name, float x, float y)
{
	uniforms.insert_or_assign(name, Uniform(glm::vec2(x,y)));
	glUniform2f(get_location(name), x, y);
}

void Program::set_vec3f(const std::string& name, const glm::vec3& value)
{
	uniforms.insert_or_assign(name, Uniform(value));
	glUniform3fv(get_location(name), 1, glm::value_ptr(value));
}

void Program::set_vec3f(const std::string& name, float r, float g, float b)
{
	uniforms.insert_or_assign(name, Uniform(glm::vec3(r,g,b)));
	glUniform3f(get_location(name), r, g, b);
}

void Program::set_mat4f(const std::string& name, const glm::mat4& value)
{
	uniforms.insert_or_assign(name, Uniform(value));
	glUniformMatrix4fv(get_location(name), 1, GL_FALSE, glm::value_ptr(value));
}
