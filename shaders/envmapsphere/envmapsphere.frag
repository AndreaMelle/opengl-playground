#version 410 core

layout (location = 0) out vec4 color;

uniform sampler2D tex_envmap;

in VS_OUT
{
  vec3 N;
  vec3 V_modelview;
} fs_in;

void main(void)
{
    vec3 V = normalize(fs_in.V_modelview);

    // reflect V about surface normal
    vec3 R = reflect(V, normalize(fs_in.N));

    // scale and bias
    R.z += 1.0;
    float m = 0.5 * inversesqrt(dot(R, R));

    color = vec4(texture(tex_envmap, R.xy * m + vec2(0.5)).rgb, 1.0);
}
