#version 330 core
out vec4 FragColor;

in vec3 w_position;
in vec3 w_normal;
in vec2 tex_coord;

uniform sampler2D color_texture;

void main()
{
	FragColor = texture(color_texture, tex_coord);
}