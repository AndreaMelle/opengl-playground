#version 410 core

layout (location = 0) out vec4 out_color;

uniform vec3 color;

in VS_OUT
{
  vec2 UV;
  vec3 vertex_color;
} fs_in;

void main(void)
{
    out_color = vec4(color * fs_in.vertex_color, 1.0);
}
