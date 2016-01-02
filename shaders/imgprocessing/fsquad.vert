#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out VS_OUT
{
    vec2 UV;
} vs_out;

uniform vec2 screen_aspect;

void main(void)
{
    gl_Position = vec4(position.x * screen_aspect.x, position.y * screen_aspect.y, 0.0, 1.0);
    vs_out.UV = uv;
}
