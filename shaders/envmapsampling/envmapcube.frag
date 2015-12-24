#version 410 core

layout (location = 0) out vec4 color;

uniform samplerCube tex_envmap;

in VS_OUT
{
  vec3 N;
  vec3 V_modelview;
} fs_in;

void main(void)
{
    vec3 V = normalize(fs_in.V_modelview);
    vec3 R = reflect(V, normalize(fs_in.N));
    color = vec4(texture(tex_envmap, R).rgb, 1.0);
}
