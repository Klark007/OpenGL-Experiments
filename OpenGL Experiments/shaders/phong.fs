#version 330 core
out vec4 FragColor;

struct Material {
	vec3 diffuse;
	vec3 specular;
	float shinniness;
};
uniform Material mat;

struct Light {
	vec3 pos;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light;

in vec3 v_normal;

void main()
{
	FragColor = vec4((v_normal+vec3(1.0,1.0,1.0)) / 2.0, 1.0);
}