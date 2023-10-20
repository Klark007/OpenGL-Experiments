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
Sphere sphere = {vec3(0.0, 0.0, -15.0), 5};

struct Ray {
	vec3 o;
	vec3 d;
};

// sun light not small point lights
vec3 light_pos = vec3(0.0, 15.0, -15.0);
vec3 light_color = vec3(1.3, 0.3, 0.9);

struct Volume {
	float absorption;
	float scattering; 
};
Volume volume = {0.1, 0.1};
float step_size = 0.2;

float jitter_str = 0.38; // range [0,1]

bool intersect_sphere(Sphere s, Ray r, out float t0, out float t1);
float linear_depth();
float gold_noise(in vec2 xy, in float seed);

float light_transmission(vec3 origin, vec3 dest) {
	Ray n = {origin, normalize(dest-origin)};

	float t0;
	float t1;

	float trans = 1.0;
	if (intersect_sphere(sphere, n, t0, t1) && t1 >= 0) {
		// increase step size for light transmission calculations
		float tau = 0.0;
		for (float t = 0; t < t1; t += step_size) {
			tau -= 1.0;
		}
		trans = exp(tau * step_size * (volume.absorption+volume.scattering));
	}
	//return exp(-t1 * (volume.absorption+volume.scattering));
	return trans;
}

vec3 raymarching(Ray r, float t0, float t1) {
	vec3 res = vec3(0);
	float transmission = 1.0; // how much of light is lost due to outscattering and absorption 

	vec3 result = vec3(0.0);
	// this introduces noise but removes Banding
	// impact of noise could be lessend by blur, avoiding small regions with much transmission or smaller perturbation
	float n = (gold_noise(gl_FragCoord.xy, 0.9787)-0.5) * jitter_str + 0.5;
	for (float t = t0+step_size*n; t < t1; t += step_size) {
		transmission *= exp(-step_size*(volume.absorption+volume.scattering)); // absorption and out scattering

		// compute in scattering from light source
		float l_transmission = light_transmission(r.o + r.d*t, light_pos);
		result += transmission * l_transmission * light_color * volume.scattering * step_size;
	}

	result += texture(frame, tex_coord).rgb * transmission;
	return result;
}

void main()
{
	Ray r = {w_pos + vec3(0,0,projection.d_near), normalize(w_dir)};
	float t0;
	float t1;

	vec4 background_color = texture(frame, tex_coord);
	if (intersect_sphere(sphere, r, t0, t1) && t1 >= 0) {
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