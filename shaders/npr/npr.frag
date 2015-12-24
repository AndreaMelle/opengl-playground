#version 410 core

layout (location = 0) out vec4 color;

in VS_OUT
{
  vec3 N;
  vec4 L_modelview;
  vec3 V_modelview;
  vec2 UV;
} fs_in;

uniform sampler2D gradientwrap_tex;
uniform sampler2D diffuse_tex;
uniform int material_mode = 1;

struct LightParams
{
  vec3 light_intensity;
  vec3 attenuation; //Constant, Linear, Exp
};

struct MaterialParams
{
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  float specular_power;
  float rim_power;
};

uniform LightParams lightParams;
uniform MaterialParams matParams;

float compute_rim_mask(vec3 N, vec3 V, float power)
{
    float rim_factor = 1.0 - dot(N, V);
    rim_factor = smoothstep(0.0, 1.0, rim_factor);
    rim_factor = pow(rim_factor, power);
    return rim_factor;
}

void main(void)
{
  float attenuation = 1.0f;

  if(fs_in.L_modelview.w != 0)
  {
    float d = length(fs_in.L_modelview.xyz);
    attenuation = lightParams.attenuation[0]
                  + lightParams.attenuation[1] * d
                  + lightParams.attenuation[2] * d * d;
  }

  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(fs_in.L_modelview.xyz);
  vec3 V = normalize(fs_in.V_modelview);
  //vec3 H = normalize(L + V);

  float wrap_coord = dot(N,L) * 0.5 + 0.5;
  wrap_coord *= wrap_coord;
  vec3 wrap_color = texture(gradientwrap_tex, vec2(wrap_coord, 0.5)).rgb;

  float diffuse = max(dot(N,L), 0.0);
  vec3 diffuse_color = texture(diffuse_tex, fs_in.UV).rgb * matParams.diffuse_color;

  float rim_mask = compute_rim_mask(N, V, matParams.rim_power);

  switch(material_mode)
  {
      case 1:
      default:
        color = vec4(wrap_color, 1.0);
        break;
      case 2:
        color = vec4(diffuse * diffuse_color, 1.0);
        break;
      case 3:
        color = vec4(vec3(rim_mask), 1.0);
        break;
      case 4:
        color = vec4(rim_mask * wrap_color + (1.0 - rim_mask) * wrap_color * diffuse_color, 1.0);
        break;
  }

  //vec3 specular = pow(max(dot(N,H), 0.0), matParams.specular_power) * matParams.specular_color;
  //color = vec4(matParams.ambient_color + lightParams.light_intensity * attenuation * (diffuse + specular), 1.0);
}
