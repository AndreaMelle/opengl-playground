#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

out VS_OUT
{
  vec4 normal;
  vec4 color;
} vs_out;

void main(void)
{
  gl_Position = position;
  vs_out.normal = normal;
  vs_out.color = vec4(0.0, 1.0, 0.0, 1.0);
}
