#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 normal;

out VS_OUT
{
    vec3 position_modelview;
    vec3 normal;
    flat uint material_id; // won't get interpolated to fragment shader
} vs_out;

layout (std140) uniform transforms
{
    mat4 modelview;
    mat4 MVP;
    mat4 normalmatrix;
};

uniform uint materialID;

void main(void)
{
    gl_Position = MVP * position;

    vs_out.position_modelview = (modelview * position).xyz;

    vs_out.normal = (normalmatrix * normal).xyz;

    vs_out.material_id = materialID;

}
