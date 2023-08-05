#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 w_position;
out vec3 w_normal;
out vec2 tex_coord;

void main() 
{
	gl_Position = projection * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
	
	// send world space vertex positions and normals
	w_position = vec3(model * vec4(pos, 1.0));
	w_normal   = mat3(transpose(inverse(model))) * normal;
	tex_coord  = uv;
}