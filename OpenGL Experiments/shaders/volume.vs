#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 uv;

out vec2 tex_coord;

void main()
{
    gl_Position = vec4(pos.x*2 - 1, pos.z*2 - 1, 0.0, 1.0); 
    tex_coord = uv;
}  