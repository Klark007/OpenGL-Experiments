#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse_0;
	sampler2D specular_0;
	float shininess;
};
uniform Material mat;

struct Light {
	vec3 pos;
	vec3 diffuse;
	vec3 specular;

	sampler2D shadow_map;
};
uniform Light light;

uniform vec3 camera_pos;

in vec3 w_position;
in vec3 w_normal;
in vec2 tex_coord;
in vec4 light_frag_pos;

// checks if light_frag_pos is shadowed by light source
// returns 1.0 if in shadow
float in_shadow() {
	vec4 shadow_tex_coord = light_frag_pos / light_frag_pos.w;
	shadow_tex_coord = shadow_tex_coord / 2 + 0.5;
	return (shadow_tex_coord.z > texture(light.shadow_map, shadow_tex_coord.xy).r) ? 1.0 : 0.0;
}

void main()
{
	vec3 ambient = vec3(texture(mat.diffuse_0, tex_coord)) * light.diffuse;

	vec3 normal    = normalize(w_normal);
	vec3 light_dir = normalize(light.pos - w_position);
	vec3 diffuse   = vec3(texture(mat.diffuse_0, tex_coord)) * max(dot(normal, light_dir), 0.0) * light.diffuse;

	vec3 view_dir  = normalize(camera_pos - w_position);
	vec3 reflected = reflect(-light_dir, normal);
	vec3 specular  = vec3(texture(mat.specular_0, tex_coord)) * pow(max(dot(reflected, view_dir), 0.0), mat.shininess) * light.specular; // reflect expects from light to vertex

	vec3 light_color = 0.5*ambient + (1.0-in_shadow()) * (diffuse + specular);
	
	FragColor = vec4(light_color, 1.0);
}