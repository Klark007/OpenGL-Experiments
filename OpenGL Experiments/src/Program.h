#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#define LOG_LENGTH 512

struct Uniform;

class Program
{
public:
	Program();
	Program(std::vector<std::shared_ptr<Shader>>& shaders);
	~Program();

	void attach_shader(unsigned int id);
	void attach_shader(Shader& shader);
	void link_program();

	// record current values of uniform using map
	void recompile();

	void use();
private:
	unsigned int program;
	char log[LOG_LENGTH];

	std::unordered_map<std::string, Uniform> uniforms;

	inline int get_location(const std::string& name) { return glGetUniformLocation(program, name.c_str()); }
public:
	void get(unsigned int name, int* params);
	void print_link_error();
	inline unsigned int get_id() { return program; };

	void set1i(const std::string& name, int value);
	void set1f(const std::string& name, float value);

	void set_vec2f(const std::string& name, const glm::vec2& value);
	void set_vec2f(const std::string& name, float x, float y);

	void set_vec3f(const std::string& name, const glm::vec3& value);
	void set_vec3f(const std::string& name, float r, float g, float b);
	
	void set_mat4f(const std::string& name, const glm::mat4& value);
};

struct Uniform {
	enum
	{
		INT,
		FLOAT,
		VEC2,
		VEC3,
		MAT4
	} tag;
	union
	{
		int i;
		float f;
		glm::vec2 v2;
		glm::vec3 v3;
		glm::mat4 m4;
	};

	explicit Uniform(int x) { tag = INT; i = x; };
	explicit Uniform(float x) {tag = FLOAT; f = x; };
	Uniform(const glm::vec2& x) { tag = VEC2; v2 = x; };
	Uniform(const glm::vec3& x) { tag = VEC3; v3 = x; };
	Uniform(const glm::mat4& x) { tag = MAT4; m4 = x; };

	inline void set(Program& p, const std::string& name) const {
		switch (tag)
		{
		case Uniform::INT:
			p.set1i(name, i);
			break;
		case Uniform::FLOAT:
			p.set1f(name, f);
			break;
		case Uniform::VEC2:
			p.set_vec2f(name, v2);
			break;
		case Uniform::VEC3:
			p.set_vec3f(name, v3);
			break;
		case Uniform::MAT4:
			p.set_mat4f(name, m4);
			break;
		}
	}
};
