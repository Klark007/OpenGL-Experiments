#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 uv;

// assume main program uses perspective projection
struct Projection {
	float y_fov;
	float d_near;
};
uniform Projection projection;

uniform vec2 resolution;

uniform mat4 view;

out vec3 w_dir;
out vec2 tex_coord;

vec2 near_plane_size() {
    float aspect_ratio = resolution.x / resolution.y;

    float len_y = tan(0.5 * projection.y_fov) * projection.d_near * 2.0;
    float len_x = aspect_ratio * len_y;
    return vec2(len_x, len_y);
}

void main()
{
    vec2 d_pos = vec2(pos.x*2 - 1, pos.z*2 - 1); // screen position between -1 and 1

    vec2 ps = near_plane_size();
    vec4 v_dir = vec4(d_pos.x * ps.x*0.5 , d_pos.y * ps.y*0.5, -projection.d_near, 0);
    w_dir = vec3(inverse(view) * v_dir);

    gl_Position = vec4(d_pos.x, d_pos.y, 0.0, 1.0); 
    tex_coord = uv;
}