#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#define LOG_LENGTH 512

class Program
{
public:
	Program();
	~Program();

	void attach_shader(Shader& shader);
	void link_program();

	void use();
private:
	unsigned int program;
	char log[LOG_LENGTH];

	inline int get_location(const char* name) { return glGetUniformLocation(program, name); }
public:
	void get(unsigned int name, int* params);
	void print_link_error();
	inline unsigned int get_id() { return program; };

	void set1i(const char* name, int value);
	void set1f(const char* name, float value);

	void set_vec3f(const char* name, glm::vec3& value);
	void set_vec3f(const char* name, float r, float g, float b);
	
	void set_mat4f(const char* name, glm::mat4& value);
};

