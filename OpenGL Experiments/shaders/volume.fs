#version 330 core
out vec4 FragColor;

in vec2 tex_coord;
uniform sampler2D frame;

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
	float t0;
	float t1;

	if (intersect_sphere(sphere, t, t0, t1) && t1 > 0.0) {
		FragColor = texture(frame, tex_coord);
	} else {
		FragColor = 1-texture(frame, tex_coord);
	}
	
}