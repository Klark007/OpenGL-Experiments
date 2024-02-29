#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
uniform sampler2D frame;
uniform sampler2D volume;

void main()
{
	vec3 fra_sample = texture(frame, tex_coord).rgb;

	vec4 vol_sample = texture(volume, tex_coord);
	vec3 vol_color = vol_sample.rgb;
	float transmission = 1 - vol_sample.a; 

	FragColor = vec4(fra_sample * transmission + vol_color, 1);
}