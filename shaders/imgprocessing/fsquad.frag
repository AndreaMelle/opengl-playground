#version 410 core

layout (location = 0) out vec4 color;

uniform sampler2D tex;
uniform sampler3D lut;
uniform vec2 scaleoffset;

struct Vignette
{
    uint apply;
    float radius;
    float softness;
    float mix_strength;
};

uniform Vignette vignette;

in VS_OUT
{
  vec2 UV;
} fs_in;

void main(void)
{
    float scale = scaleoffset[0];
    float offset = scaleoffset[1];
    vec3 input_color = texture(tex, fs_in.UV).rgb  * scale + vec3(offset);

    vec3 output_color = texture(lut, input_color).rgb;

    if(vignette.apply == 1)
    {
        vec2 pos = fs_in.UV - vec2(0.5);
        float len = length(pos);
        float v = smoothstep(vignette.radius, vignette.radius - vignette.softness, len);
        output_color = mix(output_color, output_color * v, vignette.mix_strength);
        color = vec4(output_color, 1.0);
    }
    else
    {
        color = vec4(output_color, 1.0);
    }

}
