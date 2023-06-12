#version 330 core
out vec4 FragColor;

in vec3 vert_pos;

void main()
{
	vec3 pos = vert_pos / 2;
	FragColor = vec4(pos, 1.0);
}