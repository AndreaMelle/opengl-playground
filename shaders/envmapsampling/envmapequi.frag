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

    vec2 tc;
    tc.y = R.y;
    R.y = 0.0;
    tc.x = normalize(R).x * 0.5;
    float s = sign(R.z) * 0.5;

    tc.s = 0.75 - s * (0.5 - tc.s);
    tc.t = 0.5 + 0.5 * tc.t;

    color = vec4(texture(tex_envmap, tc).rgb, 1.0);
}
