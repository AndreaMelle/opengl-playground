#version 410 core

// we are writing to two color attachments.
layout (location = 0) out uvec4 color0;
layout (location = 1) out vec4 color1;

in VS_OUT
{
    vec3 position_modelview;
    vec3 normal;
    flat uint material_id;
} fs_in;

struct MaterialParams
{
  vec3 diffuse_color;
  float specular_power;
};

uniform MaterialParams matParams;

#define MAX_SHORT_F 65535.0f

uint packHalf2x16(vec2 v)
{
    uint packed;
    packed  = uint(v.x * MAX_SHORT_F + 0.5f);
    packed |= uint(v.y * MAX_SHORT_F  + 0.5f) << 16;
    return packed;
}

void main(void)
{
    uvec4 outvec0 = uvec4(0);
    vec4 outvec1 = vec4(0);

    // we need to pack:
    // 3 x 16-bit (half) color components
    // 3 x 16-bit (half) normal components
    // 1 x 32-bit (full) material id component
    // 3 x 32-bit (full) position components
    // 1 x 32-bit (full) specular component

    // These quantities are all unsigned

    vec3 n = normalize(fs_in.normal) * 0.5 + 0.5;

    outvec0.x = packHalf2x16(matParams.diffuse_color.xy);
    outvec0.y = packHalf2x16(vec2(matParams.diffuse_color.z, n.x));
    outvec0.z = packHalf2x16(n.yz);
    outvec0.w = fs_in.material_id;

    // eye space position is signed
    outvec1.xyz = fs_in.position_modelview;
    outvec1.w = matParams.specular_power;

    color0 = outvec0;
    color1 = outvec1;
}
