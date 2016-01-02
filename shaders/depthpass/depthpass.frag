#version 410 core

layout (location = 0) out float fragdepth;

void main(void)
{
    // can't we just grab the damn depth buffer??
    fragdepth = gl_FragCoord.z;
}
