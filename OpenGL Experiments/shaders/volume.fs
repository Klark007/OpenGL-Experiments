#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
in vec3 w_dir;

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
Sphere sphere = {vec3(0.0, 0.0, -10.0), 2.5};

struct Ray {
	vec3 o;
	vec3 d;
};
Ray t = {vec3(-5.0, 2.4, 0.0), vec3(1.0, 0.0, 0.0)};

float linear_depth();

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

void main()
{
	Ray r = {w_pos + vec3(0,0,projection.d_near), normalize(w_dir)};
	float t0;
	float t1;

	/*
	float frame_depth = linear_depth() / 100;
	FragColor = vec4(frame_depth, frame_depth, frame_depth, 1);
	return;
	*/
	
	if (intersect_sphere(sphere, r, t0, t1) && t0 >= 0) {
		float frame_depth = linear_depth();

		vec4 intersect_pos = vec4(t0 * r.d, 0);

		float ray_depth = abs((view*intersect_pos).z);
		if (ray_depth < frame_depth) {
			FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
		}
	}
	
	FragColor = texture(frame, tex_coord);
}

// z value i.e. distance to camera plane
// based on https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer/6657284#6657284
float linear_depth() {

	float z_b = texture(depth, tex_coord).r;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * projection.d_near * projection.d_far / (projection.d_far + projection.d_near - z_n * (projection.d_far - projection.d_near));
	return z_e;
}