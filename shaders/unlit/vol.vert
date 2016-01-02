#version 410 core

layout (location = 0) in vec4 position;

uniform mat4 MVP;

out VS_OUT
{
  vec3 position_model;
} vs_out;

void main(void)
{
  vs_out.position_model = position.xyz;
  gl_Position = MVP * position;
}
