#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

layout (std140) uniform constants
{
  mat4 modelview;
  mat4 MVP;
  mat4 normalmatrix;
  mat4 shadowmapMVP;
};

uniform vec4 light_pos; //eye space

out VS_OUT
{
  vec3 N;
  vec4 L_modelview;
  vec3 V_modelview;
  vec4 P_shadowmap;
} vs_out;

void main(void)
{
  gl_Position = MVP * position;
  vec3 P_modelview = (modelview * position).xyz;

  vs_out.N = (normalmatrix * normal).xyz;

  vs_out.V_modelview = -P_modelview;

  if(light_pos.w == 0)
  {
    vs_out.L_modelview = vec4(-light_pos.xyz, 0);
  }
  else
  {
    vs_out.L_modelview = vec4(P_modelview - light_pos.xyz, light_pos.w);
  }

  vs_out.P_shadowmap = shadowmapMVP * vec4(position.xyz, 1.0);

}
