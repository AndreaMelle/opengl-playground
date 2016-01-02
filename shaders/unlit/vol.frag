#version 410 core

layout (location = 0) out vec4 color;

uniform sampler3D tex;

in VS_OUT
{
  vec3 position_model;
} fs_in;

void main(void)
{
    vec3 uv = fs_in.position_model;
    uv.z = uv.z * 1.5f;
    //uv.y = uv.y * 0.42578125;
    float intensity = texture(tex, uv).r;

    if(intensity < 0.2f)
        discard;

    color = vec4(intensity, intensity, intensity, intensity);
}
