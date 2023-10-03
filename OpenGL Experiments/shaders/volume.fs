#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
uniform sampler2D frame;

// assume main program uses perspective projection
struct Projection {
	float y_fov;
	float d_near;
};
uniform Projection projection;

uniform mat4 view;
uniform vec2 resolution;

struct Sphere {
	vec3 p;
	float r;
};
Sphere sphere = {vec3(0.0, 0.0, 0.0), 2.5};

struct Ray {
	vec3 o;
	vec3 d;
};
Ray t = {vec3(-5.0, 2.51, 0.0), vec3(1.0, 0.0, 0.0)};

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

	float x_0 = -b + sqrt(d) / (2*a);
	float x_1 = -b - sqrt(d) / (2*a);

	t0 = min(x_0, x_1);
	t1 = max(x_0, x_1);

	return true;
}

void main()
{
	// precomputation could be moved to fragment shader
	float aspect_ratio = resolution.x / resolution.y;
	vec3 r_origin = vec3(0, projection.d_near, 0);

	// dimension of the near plane defined by fov, aspect ratio and distance
	float len_y = tan(projection.y_fov / 2.0) * projection.d_near * 2.0;
	float len_x = aspect_ratio * len_y;

	// lower left corner of near plane offset to be at center of pixel
	float i_res_x = 1.0 / resolution.x;
	float i_res_y = 1.0 / resolution.y;

	vec3 corner = vec3(-len_x/2.0, 0, -len_y/2.0) + vec3(0.5 * i_res_x, 0, 0.5 * i_res_y);

	vec3 r_dest = corner + vec3(tex_coord.x * len_x, 0, tex_coord.y * len_y);

	vec3 color = r_dest + vec3(len_x/2.0, 0, len_y/2.0);
	FragColor = vec4(color.r/len_x, color.b/len_y, 0, 1.0);	
}