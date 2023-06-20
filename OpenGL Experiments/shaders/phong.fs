#version 330 core
out vec4 FragColor;

struct Material {
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material mat;

struct Light {
	vec3 pos;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light;

uniform vec3 camera_pos;

in vec3 w_position;
in vec3 w_normal;

void main()
{
	vec3 ambient = mat.diffuse * light.diffuse;

	vec3 normal    = normalize(w_normal);
	vec3 light_dir = normalize(light.pos - w_position);
	vec3 diffuse   = mat.diffuse * max(dot(normal, light_dir), 0.0) * light.diffuse;

	vec3 view_dir  = normalize(camera_pos - w_position);
	vec3 reflected = reflect(-light_dir, normal);
	vec3 specular  = mat.specular * pow(max(dot(reflected, view_dir), 0.0), mat.shininess) * light.specular; // reflect expects from light to vertex

	vec3 light_color = 0.5*ambient + diffuse + specular;
	FragColor = vec4(light_color, 1.0); // vec4((w_normal+vec3(1.0,1.0,1.0)) / 2.0, 1.0);
}