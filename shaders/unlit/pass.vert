#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec3 color;

uniform mat4 MVP;

out VS_OUT
{
  vec2 UV;
  vec3 vertex_color;
} vs_out;

void main(void)
{
  gl_Position = MVP * position;
  vs_out.UV = uv;
  vs_out.vertex_color = color;
}
