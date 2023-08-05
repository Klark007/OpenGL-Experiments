#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse_texture;
	sampler2D specular_texture;
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
in vec2 tex_coord;


void main()
{
	vec3 ambient = vec3(texture(mat.diffuse_texture, tex_coord)) * light.diffuse;

	vec3 normal    = normalize(w_normal);
	vec3 light_dir = normalize(light.pos - w_position);
	vec3 diffuse   = vec3(texture(mat.diffuse_texture, tex_coord)) * max(dot(normal, light_dir), 0.0) * light.diffuse;

	vec3 view_dir  = normalize(camera_pos - w_position);
	vec3 reflected = reflect(-light_dir, normal);
	vec3 specular  = vec3(texture(mat.specular_texture, tex_coord)) * pow(max(dot(reflected, view_dir), 0.0), mat.shininess) * light.specular; // reflect expects from light to vertex

	vec3 light_color = 0.5*ambient + diffuse + specular;
	
	FragColor = vec4(light_color, 1.0); // vec4((w_normal+vec3(1.0,1.0,1.0)) / 2.0, 1.0);
}