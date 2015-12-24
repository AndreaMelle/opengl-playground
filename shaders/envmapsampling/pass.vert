#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

layout (std140) uniform constants
{
  mat4 modelview;
  mat4 MVP;
  mat4 normalmatrix;
};

out VS_OUT
{
  vec3 N;
  vec3 V_modelview;
} vs_out;

void main(void)
{
  gl_Position = MVP * position;
  vs_out.N = (normalmatrix * normal).xyz;
  vs_out.V_modelview = (modelview * position).xyz;
}
