#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <vector>
#include <memory>
#include <string>

#include <unordered_map>
#include <unordered_set>

#define LOG_LENGTH 512
#define UPDATE_UNIFORM_IF_CHANGED true

struct Uniform;

class Program
{
public:
	Program();
	Program(std::vector<std::shared_ptr<Shader>>& shaders);
	~Program();

	void attach_shader(std::shared_ptr<Shader> shader);
	void link_program();

	// record current values of uniform using map
	void recompile();

	void use();
private:
	unsigned int program;
	char log[LOG_LENGTH];

	std::unordered_set<std::shared_ptr<Shader>> shaders;
	std::unordered_map<std::string, Uniform> uniforms;

	inline int get_location(const std::string& name) { return glGetUniformLocation(program, name.c_str()); }
	inline void update_uniform(const std::string& name, Uniform u);
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
	} u;

	explicit Uniform(int x) { tag = INT; u.i = x; };
	explicit Uniform(float x) {tag = FLOAT; u.f = x; };
	Uniform(const glm::vec2& x) { tag = VEC2; u.v2 = x; };
	Uniform(const glm::vec3& x) { tag = VEC3; u.v3 = x; };
	Uniform(const glm::mat4& x) { tag = MAT4; u.m4 = x; };

	friend bool operator== (const Uniform& a, const Uniform& b) { return a.tag == b.tag && a.u.i == b.u.i; }
	friend bool operator!= (const Uniform& a, const Uniform& b) { return !(a == b); }

	inline void set(Program& p, const std::string& name) const {
		switch (tag)
		{
		case Uniform::INT:
			p.set1i(name, u.i);
			break;
		case Uniform::FLOAT:
			p.set1f(name, u.f);
			break;
		case Uniform::VEC2:
			p.set_vec2f(name, u.v2);
			break;
		case Uniform::VEC3:
			p.set_vec3f(name, u.v3);
			break;
		case Uniform::MAT4:
			p.set_mat4f(name, u.m4);
			break;
		}
	}
};

inline void Program::update_uniform(const std::string& name, Uniform u)
{
#if UPDATE_UNIFORM_IF_CHANGED
	if (uniforms.find(name) != uniforms.end() && uniforms.at(name) == u) {
		return;
	}
#endif
	uniforms.insert_or_assign(name, u);
}