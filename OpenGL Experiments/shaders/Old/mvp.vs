#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vert_pos;

void main() 
{
	gl_Position = projection * view * model * vec4(pos.x, pos.y, pos.z, 1.0);
	vert_pos = gl_Position.www;
}