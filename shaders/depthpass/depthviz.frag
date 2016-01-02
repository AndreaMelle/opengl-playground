#version 410 core

layout (location = 0) out vec4 color;

uniform sampler2D depth_buffer;

void main(void)
{
    float d = texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).x;
    float near = gl_DepthRange.near;//0.1;
    float far = gl_DepthRange.far;//10.0;
    float depth = d;//1.0 - ((2.0 * near) / (far + near - d * (far - near)));
    color = vec4(depth, depth, depth, 1.0);
}
