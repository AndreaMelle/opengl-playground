#version 410 core

layout (location = 0) out vec4 color;

in VS_OUT
{
  vec3 N;
  vec4 L_modelview;
  vec3 V_modelview;
} fs_in;

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
};

uniform LightParams lightParams;
uniform MaterialParams matParams;

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

  vec3 R = reflect(-L, N);

  vec3 diffuse = max(dot(N,L), 0.0) * matParams.diffuse_color;
  vec3 specular = pow(max(dot(R,V), 0.0), matParams.specular_power) * matParams.specular_color;

  color = vec4(matParams.ambient_color + lightParams.light_intensity * attenuation * (diffuse + specular), 1.0);
}
