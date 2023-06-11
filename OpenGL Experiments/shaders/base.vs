#version 330 core
layout (location = 0) in vec3 pos;

uniform float offset;

out vec3 oPos;

void main() 
{
	gl_Position = vec4(pos.x+offset, pos.y, pos.z, 1.0);
	oPos = pos;
}