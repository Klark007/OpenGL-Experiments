#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse_0;
	sampler2D specular_0;
	float shininess;
};
uniform Material mat;

#define NR_SHADOW_SAMPLES 16
struct Light {
	vec3 pos;
	vec3 diffuse;
	vec3 specular;

	sampler2D shadow_map;
	vec2 shadow_samples[NR_SHADOW_SAMPLES];
	vec2 occluder_sample;
	float width;

	int shadow_mode;
	int pcss;
};
uniform Light light;

uniform vec3 camera_pos;

in vec3 w_position;
in vec3 w_normal;
in vec2 tex_coord;
in vec4 light_frag_pos;

float gold_noise(in vec2 xy, in float seed);
mat3 rotate_z(float theta);

float occluder_distance(vec2 tex_c) {
	 return texture(light.shadow_map, tex_c.xy).r;
}

// checks if light_frag_pos is shadowed by light source
// returns 1.0 if in shadow
float in_shadow(vec3 tex_c) {
	
	return (tex_c.z > occluder_distance(tex_c.xy)) ? 1.0 : 0.0;
}

float in_hard_shadow() {
	vec4 shadow_tex_coord = light_frag_pos / light_frag_pos.w;
	shadow_tex_coord = shadow_tex_coord / 2 + 0.5;

	return in_shadow(shadow_tex_coord.xyz);
}

float pcf(vec4 shadow_coord, float width, vec2 texel_size) {
	float soft_shadow_average = 0.0;
	
	if (light.shadow_mode == 0) {
		soft_shadow_average = in_shadow(shadow_coord.xyz) * NR_SHADOW_SAMPLES;
	} else if (light.shadow_mode == 1) {
		// samples regular grid from -width/2 to width/2 from center of pixel in both x and y
		float step_size = width / (sqrt(float(NR_SHADOW_SAMPLES))-1);

		// add step_size/2 to account for small floating point error, could also be something like 1e-5
		for (float y = -width/2; y <= width/2+step_size/2; y += step_size) {
			for (float x = -width/2; x <= width/2+step_size/2; x += step_size) {
				soft_shadow_average += in_shadow(shadow_coord.xyz + vec3(x * texel_size.x,y * texel_size.y,0));
			}
		}
	} else if (light.shadow_mode == 2) {
		for (int i = 0; i < NR_SHADOW_SAMPLES; i++) {
			soft_shadow_average += in_shadow(shadow_coord.xyz + vec3((light.shadow_samples[i].x*width - width/2) * texel_size.x,(light.shadow_samples[i].y*width - width/2) * texel_size.y,0));
		}
	} else if (light.shadow_mode == 3) {
		for (int i = 0; i < NR_SHADOW_SAMPLES; i++) {
			vec3 offset = vec3((light.shadow_samples[i].x*width - width/2) * texel_size.x,(light.shadow_samples[i].y*width - width/2) * texel_size.y,0);

			float seed = gold_noise(gl_FragCoord.xy, 3.74687876);
			mat3 rotation = rotate_z(mod(seed, 2*3.14159));
			soft_shadow_average += in_shadow(shadow_coord.xyz + rotation*offset);
		}
	}
	
	return soft_shadow_average / NR_SHADOW_SAMPLES;
}

float sample_occluder(vec2 tex_c, vec2 texel_size) {
	vec2 tex_coord = tex_c + light.occluder_sample * texel_size;
	return occluder_distance(tex_coord);
}

float in_soft_shadow() {
	vec4 shadow_tex_coord = light_frag_pos / light_frag_pos.w;
	shadow_tex_coord = shadow_tex_coord / 2 + 0.5;
	vec2 texel_size = 1.0 / textureSize(light.shadow_map, 0);

	float d_O = sample_occluder(shadow_tex_coord.xy, texel_size);
	float d_R = shadow_tex_coord.z;

	float width;
	if (light.pcss == 1) {
		if (d_R <= d_O) {
			return 0.0;
		}

		width = (d_R - d_O) * light.width / d_O;
	} else {
		width = 3.0;
	}


	return pcf(shadow_tex_coord, width, texel_size);
}

float width_debugging() {
	vec4 shadow_tex_coord = light_frag_pos / light_frag_pos.w;
	shadow_tex_coord = shadow_tex_coord / 2 + 0.5;
	vec2 texel_size = 1.0 / textureSize(light.shadow_map, 0);

	float d_O = sample_occluder(shadow_tex_coord.xy, texel_size);
	float d_R = shadow_tex_coord.z;

	if (d_R <= d_O) {
		return 0.0;
	}

	float width = (d_R - d_O) * light.width / d_O;
	return width;
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

	vec3 light_color = 0.2*ambient + (1.0-in_soft_shadow()) * (diffuse + specular);
	
	//FragColor = vec4(width_debugging() / 10.0, 0.0, 0.0, 1.0);
	FragColor = vec4(light_color, 1.0);
}

float PHI = 1.61803398874989484820459; 
float gold_noise(in vec2 xy, in float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

mat3 rotate_z(float theta) {
	mat3 res; // access column first
	res[0] = vec3(cos(theta), sin(theta), 0);
	res[1] = vec3(-sin(theta), cos(theta), 0);
	res[2][2] = 1.0;
	return res;
}