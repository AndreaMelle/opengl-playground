#version 410 core

layout (location = 0) in vec4 position; //model space vertex position
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

layout (std140) uniform constants
{
  mat4 modelview;
  mat4 MVP;
  mat4 normalmatrix;
};

// Normal and world space light vector
out VS_OUT
{
  vec3 N;
  vec3 position_view;
} vs_out;

void main(void)
{
  // vertex in world space
  vs_out.position_view = (modelview * position).xyz;

  // Assume normal.w = 0
  // This is correct ONLY if model matrix does not scale the vertices
  // Otherwise we MUST use the inverse transpose of model matrix
  //vs_out.N = (modelview * normal).xyz;
  vs_out.N = (normalmatrix * normal).xyz;
  //vs_out.N = (transpose(inverse(modelview)) * vec4(normal.xyz, 0)).xyz;
  gl_Position = MVP * position;
}
