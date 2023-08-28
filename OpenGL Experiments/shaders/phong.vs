#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_space;

out vec3 w_position;
out vec3 w_normal;
out vec2 tex_coord;
out vec4 light_frag_pos;

void main() 
{
	// send world space vertex positions and normals
	w_position = vec3(model * vec4(pos, 1.0));
	w_normal   = mat3(transpose(inverse(model))) * normal;
	tex_coord  = uv;
	light_frag_pos = light_space * vec4(w_position, 1.0);

	gl_Position = projection * view * vec4(w_position, 1.0);
}