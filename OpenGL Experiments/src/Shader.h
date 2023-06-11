#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define LOG_LENGTH 512


class Shader
{
public:
	Shader(unsigned int shader_type);
	~Shader();

	int  add_source_from_file(const char* path);
	void compile();

private:
	unsigned int shader;
	char log[LOG_LENGTH];

public:
	void get(unsigned int name, int* params);
	void print_compile_error();
	inline unsigned int get_id() { return shader; };
};

