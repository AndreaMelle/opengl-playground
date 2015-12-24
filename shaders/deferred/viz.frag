#version 410 core

layout (location = 0) out vec4 color;

uniform usampler2D gbuffer_tex0;
uniform sampler2D gbuffer_tex1;
uniform sampler2D gbuffer_depth;

uniform int viz_mode = 1;

struct fragment_info_t
{
    vec3  diffuse_color;
    vec3  normal;
    float specular_power;
    vec3  position_modelview;
    uint  material_id;
    float depth;
};

#define MAX_SHORT_F 65535.0f

vec2 unpackHalf2x16(uint packed)
{
    vec2 v;
    v.x = float(packed & uint(0xFFFF)) / MAX_SHORT_F;
    v.y = float(packed >> 16) / MAX_SHORT_F;
    return v;
}

void unpack_gbuffer(ivec2 coord, out fragment_info_t frag)
{
    // bypass filtering and access texel directly
    // this call is texture size dependent, no [0, 1] normalization
    uvec4 data0 = texelFetch(gbuffer_tex0, ivec2(coord), 0);
    vec4 data1 = texelFetch(gbuffer_tex1, ivec2(coord), 0);
    vec2 temp;

    temp = unpackHalf2x16(data0.y);
    frag.diffuse_color = vec3(unpackHalf2x16(data0.x), temp.x);

    vec3 n = vec3(temp.y, unpackHalf2x16(data0.z));

    frag.normal = normalize((n - 0.5) * 2.0);

    frag.material_id = data0.w;

    frag.position_modelview = data1.xyz;
    frag.specular_power = data1.w;

    float d = texelFetch(gbuffer_depth, ivec2(coord), 0).x;
    float near = 0.1;
    float far = 10.0;
    frag.depth = 1.0 - ((2.0 * near) / (far + near - d * (far - near)));
}

vec4 compute_viz(fragment_info_t frag)
{
    vec4 result = vec4(0.0);

    switch(viz_mode)
    {
        case 1:
        default:
          result = vec4(frag.normal * 0.5 + vec3(0.5), 1.0); //[0,1]
          break;
        case 2:
          result = vec4(frag.position_modelview * 0.5 + vec3(0.5, 0.5, 0.0), 1.0);
          break;
        case 3:
          result = vec4(frag.diffuse_color, 1.0);
          break;
        case 4:
          result = vec4(frag.specular_power / 255.0,
                        float(frag.material_id & uint(15)) / 15.0,
                        float(frag.material_id / 16) / 15.0,
                        1.0);
          break;
        case 5:

          result = vec4(frag.depth, frag.depth, frag.depth, 1.0);
          break;

    }

    return result;
}

void main(void)
{
    fragment_info_t frag;
    unpack_gbuffer(ivec2(gl_FragCoord.xy), frag);
    color = compute_viz(frag);
}
