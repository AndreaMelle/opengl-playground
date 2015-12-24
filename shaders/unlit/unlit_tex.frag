#version 410 core

layout (location = 0) out vec4 color;

uniform sampler2D tex;

in VS_OUT
{
  vec2 UV;
} fs_in;

void main(void)
{
    color = vec4(texture(tex, fs_in.UV).rgb, 1.0);
}
