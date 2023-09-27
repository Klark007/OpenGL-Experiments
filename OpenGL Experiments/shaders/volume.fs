#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
uniform sampler2D frame;

void main()
{
	FragColor = texture(frame, tex_coord);
}