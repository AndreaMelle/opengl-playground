#version 410 core

layout (location = 0) out vec4 color;

uniform usampler2D gbuffer_tex0;
uniform sampler2D gbuffer_tex1;

struct LightParams
{
    vec4 light_pos; //eye space
    vec3 light_intensity;
    vec3 attenuation; //Constant, Linear, Exp
};

uniform int num_lights = 64;

layout (std140) uniform light_block
{
    LightParams light[64];
};

struct fragment_info_t
{
    vec3  diffuse_color;
    vec3  normal;
    float specular_power;
    vec3  position_modelview;
    uint  material_id;
};

#define MAX_SHORT_F 65535.0f

vec2 unpackHalf2x16(uint packed)
{
    vec2 v;
    v.x = float(packed & uint(0xFFFF)) / MAX_SHORT_F;
    v.y = float(packed >> 16) / MAX_SHORT_F;
    return v;
}

vec4 compute_light(fragment_info_t frag)
{
    int i = 0;
    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 L;
    float attenuation;

    // when 0, this fragment has no object!!
    if(frag.material_id != 0)
    {
        for(i = 0; i < num_lights; i++)
        {
            attenuation = 1.0f;

            if(light[i].light_pos.w == 0)
            {
                L = normalize(-light[i].light_pos.xyz);
            }
            else
            {
                L = frag.position_modelview - light[i].light_pos.xyz;
                float d = length(L);
                L = normalize(L);
                attenuation = light[i].attenuation[0]
                              + light[i].attenuation[1] * d
                              + light[i].attenuation[2] * d * d;
            }

            vec3 N = frag.normal;
            vec3 V = normalize(-frag.position_modelview);

            vec3 R = reflect(-L, N);

            vec3 diffuse = max(dot(N,L), 0.0) * frag.diffuse_color;
            vec3 specular = pow(max(dot(R,V), 0.0), frag.specular_power) * vec3(1.0);

            // ambient color can be passed as uniform!!
            result += vec4(light[i].light_intensity * attenuation * (diffuse + specular), 1.0);

        }

        result += vec4(0.1, 0.1, 0.1, 0.0);
    }
    else
    {
      result = vec4(0.5, 0.5, 0.5, 1.0);
    }

    return result;
}

// coords are in 0,0 -> tex_w, tex_h coord system, in pixels!
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
}

void main(void)
{
    fragment_info_t frag;
    unpack_gbuffer(ivec2(gl_FragCoord.xy), frag);
    color = compute_light(frag);
}
