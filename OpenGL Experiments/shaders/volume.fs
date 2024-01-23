// TODO
// Banding (X)
// In scattering
// Scattering distributions
// Non uniform density generation
// Performance (resolution, stepsize)
// Shadows?

#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
in vec3 w_dir;

uniform float time;

struct Projection {
	float y_fov;
	float d_near;
	float d_far; // for linearlizing depth
};
uniform Projection projection;

uniform sampler2D frame;
uniform sampler2D depth;
uniform vec3 w_pos;

uniform mat4 view;

struct Sphere {
	vec3 p;
	float r;
};
Sphere sphere = {vec3(0.0, 11.5, -15.0), 10};

struct AABB {
	vec2 min_max[3];
};
AABB cloud_bounding_box = {vec2[](vec2(-30, 30), vec2(10, 13), vec2(-30, 30))};

vec2 cloud_min_max_height = vec2(10.0,13.0);

struct Ray {
	vec3 o;
	vec3 d;
};

// sun light not small point lights
vec3 light_pos = vec3(0.0, 20.0, 20.0);
uniform vec3 light_color;
uniform float light_strength;

struct Volume {
	float absorption;
	float scattering; 
};
Volume volume = {0.1, 0.1};
uniform int nr_steps;

uniform float jitter_str = 0.9; // range [0,1]

uniform float base_cloud_translation;

uniform sampler3D worley_n;
uniform sampler2D weather_map;
uniform float global_density;
uniform int only_worley_perlin;
uniform float global_coverage;

uniform int worley_channel;
uniform vec3 worley_offset;

bool intersect_sphere(Sphere s, Ray r, out float t0, out float t1);
bool intersect_aabb(AABB box, Ray r, out float t0, out float t1);
float linear_depth();
float gold_noise(in vec2 xy, in float seed);
float remap(float t, float old_min, float old_max, float new_min, float new_max);

float get_height_fraction(float y) {
	float f = (y-cloud_min_max_height.x) / (cloud_min_max_height.y - cloud_min_max_height.x);
	return clamp(f,0,1);
}

float shape_altering_height_function(vec3 p) {
	float f = get_height_fraction(p.y);
	float SA_b = clamp(remap(f,0,0.7,0,1),0,1);
	float SA_t = clamp(remap(f,0.2,1,1,0),0,1);
	return SA_b * SA_t;
}

float density_altering_height_function(vec3 p) {
	float f = get_height_fraction(p.y);
	float DA_b = f * clamp(remap(f,0,0.15,0,1),0,1);
	float DA_t = clamp(remap(f,0.9,1,1,0),0,1);
	return DA_b * DA_t * 3.5;
}

float weather_map_sample(vec3 p) {
	vec3 weather_sample =  texture(weather_map, p.xz * 0.06 + worley_offset.xy).rgb;
	float hc_sample = weather_sample.r;
	return clamp(global_coverage-0.5, 0, 1) * hc_sample * 2;
}

float low_freq(vec3 p) {
	vec4 low_freq_noise = texture(worley_n,0.1*p);

	vec3 fbm_weights = vec3(0.625, 0.25, 0.125);
	float low_freq_fbm = dot(low_freq_noise.gba, fbm_weights);
	if (only_worley_perlin == 1) {
		low_freq_fbm = 0.0;
	}
	return clamp(remap(low_freq_noise.r, low_freq_fbm-1, 1.0, 0.0, 1.0) + base_cloud_translation, 0, 1);
}

float sample_density(vec3 p) {
	if (p.x < cloud_bounding_box.min_max[0].x || p.x > cloud_bounding_box.min_max[0].y || p.y < cloud_bounding_box.min_max[1].x || p.y > cloud_bounding_box.min_max[1].y || p.z < cloud_bounding_box.min_max[2].x || p.z > cloud_bounding_box.min_max[2].y) {
		return 0.0;
	}

	float base_cloud = low_freq(p);

	float shape_altering = shape_altering_height_function(p);
	float density_altering = density_altering_height_function(p);
	float wm = weather_map_sample(p);

	base_cloud *= shape_altering;
	base_cloud = clamp(remap(base_cloud, 1-global_coverage*wm, 1, 0, 1), 0, 1) * density_altering;
	return base_cloud * global_density;
}

float light_transmission(vec3 origin, vec3 dest) {
	Ray n = {origin, normalize(dest-origin)};

	float t0;
	float t1;

	float trans = 1.0;
	if (intersect_aabb(cloud_bounding_box, n, t0, t1) && t1 >= 0) {
		float step_size = (t1-t0) / nr_steps * 2;

		// increase step size for light transmission calculations
		float tau = 0.0;
		float t = 0;
		for (int c = 0; c < nr_steps/2; c++) {
			if (!(t < t1)) {
				break;
			}
			vec3 p = n.o + n.d*t;
			tau += sample_density(p);

			t += step_size;
		}
		trans = exp(-tau * step_size * (volume.absorption+volume.scattering));
	}
	//return exp(-t1 * (volume.absorption+volume.scattering));
	return trans;
}

vec3 raymarching(Ray r, float t0, float t1) {
	vec3 res = vec3(0);
	float transmission = 1.0; // how much of light is lost due to outscattering and absorption 
	float step_size = (t1-t0) / nr_steps;

	vec3 result = vec3(0.0);
	// this introduces noise but removes Banding
	// impact of noise could be lessend by blur, avoiding small regions with much transmission or smaller perturbation
	float n = (gold_noise(gl_FragCoord.xy, 0.9787)-0.5) * jitter_str + 0.5;
	float t = t0+step_size*n;
	for (int c = 0; c < nr_steps; c++) {
		if (!(t < t1)) {
			break;
		}
		vec3 p = r.o + t*r.d;
		float density = sample_density(p);
		
		transmission *= exp(-density*step_size*(volume.absorption+volume.scattering)); // absorption and out scattering
		
		if (density > 0) {
			// compute in scattering from light source
			float l_transmission = light_transmission(r.o + r.d*t, light_pos);
			result += transmission * (l_transmission * light_color * light_strength) * volume.scattering * step_size * density;
		}

		t += step_size;
	}
	
	result += texture(frame, tex_coord).rgb * transmission;
	return result;
}

void main()
{
	vec4 s = texture(worley_n, vec3(tex_coord, 0)+worley_offset);
	if (worley_channel == 0) {
		FragColor = vec4(s.ggg,1);
		return;
	}// else if (worley_channel == 1) {
	//	FragColor = vec4(s.ggg,1);
	//	return;
	//} else if (worley_channel == 2) {
	//	FragColor = vec4(s.bbb,1);
	//	return;
	//} else if (worley_channel == 3) {
	//	FragColor = vec4(s.aaa,1);
	//	return;
	//}
	

	Ray r = {w_pos + vec3(0,0,projection.d_near), normalize(w_dir)};
	float t0;
	float t1;

	vec4 background_color = texture(frame, tex_coord);
	if (intersect_aabb(cloud_bounding_box, r, t0, t1) && t1 >= 0) {
		bool inside = (t0 < 0);

		float frame_depth = linear_depth();
		float t = inside ? t1 : t0;
		vec4 intersect_pos = view * vec4(t * r.d, 0);

		float ray_depth = abs((intersect_pos).z);
		// depth testing
		if (ray_depth >= frame_depth) {
			FragColor = background_color;
			return;
		}
		if (inside) {
			FragColor = vec4(raymarching(r, 0, t), 1.0);
		} else {
			/*
			if (worley_channel == 1) {
				FragColor = vec4(global_coverage*weather_map_sample(r.o+r.d*t0));
				return;
			} else if (worley_channel == 2) {
				FragColor = vec4(low_freq(r.o+r.d*t0));
				return;
			}
			if (worley_channel == 3) {
				FragColor = vec4(vec3(sample_density(r.o+r.d*t0)),1);
				return;
			}
			*/

			FragColor = vec4(raymarching(r, t0, t1), 1.0);
		}
	} else {
		FragColor = background_color;
	}
}

// z value i.e. distance to camera plane
// based on https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer/6657284#6657284
float linear_depth() {

	float z_b = texture(depth, tex_coord).r;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * projection.d_near * projection.d_far / (projection.d_far + projection.d_near - z_n * (projection.d_far - projection.d_near));
	return z_e;
}

// returns true if hits sphere and records parameters of first and second hit in t0, t1
bool intersect_sphere(Sphere s, Ray r, out float t0, out float t1) {
	float a = dot(r.d, r.d);

	if (abs(a) < 1e-5) {
		return false;
	}

	float b = 2*(dot(r.o,r.d) - dot(r.d,s.p));
	float c = dot(r.o-s.p, r.o-s.p) - s.r*s.r;

	float d = b*b - 4*a*c;

	if (d < 0) {
		return false;
	}

	float x_0 = (-b + sqrt(d)) / (2*a);
	float x_1 = (-b - sqrt(d)) / (2*a);

	t0 = min(x_0, x_1);
	t1 = max(x_0, x_1);

	return true;
}

bool intersect_aabb(AABB box, Ray r, out float t0, out float t1) {
	vec3 rec_dir = 1.0 / r.d;

	vec2 t_x = (box.min_max[0] - r.o.x) * rec_dir.x;
	vec2 t_y = (box.min_max[1] - r.o.y) * rec_dir.y;
	vec2 t_z = (box.min_max[2] - r.o.z) * rec_dir.z;

	float t_min = max(max(min(t_x.x, t_x.y), min(t_y.x, t_y.y)), min(t_z.x, t_z.y));
	float t_max = min(min(max(t_x.x, t_x.y), max(t_y.x, t_y.y)), max(t_z.x, t_z.y));

	if (t_min > t_max) {
		return false;
	}

	t0 = t_min;
	t1 = t_max;

	return true;
}

float remap(float t, float old_min, float old_max, float new_min, float new_max) {
	return new_min + ((t-old_min) / (old_max-old_min) * (new_max-new_min));
}


float PI  = 3.14159265358979323846264;
float PHI = 1.61803398874989484820459; 
float gold_noise(in vec2 xy, in float seed){
	// from 
	if (mod(distance(xy*PHI, xy)*seed, PI/2.0) < 1e-5 || PI/2.0 - mod(distance(xy*PHI, xy)*seed, PI/2.0) < 1e-5) {
		return fract(tan(distance(xy*PHI, xy)*seed + PI/4.0)*xy.x);
	} else {
		return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
	}
}