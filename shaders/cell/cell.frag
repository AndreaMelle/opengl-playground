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

float stepmix(float edge0, float edge1, float E, float x)
{
    float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
    return mix(edge0, edge1, T);
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

  vec3 H = normalize(L + V);

  float df = max(dot(N,L), 0.0);
  float sf = pow(max(dot(N,H), 0.0), matParams.specular_power);

  const float A = 0.1;
  const float B = 0.3;
  const float C = 0.6;
  const float D = 1.0;

  float E = fwidth(df);

  if (df > A - E && df < A + E) df = stepmix(A, B, E, df);
  else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
  else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
  else if (df < A) df = 0.0;
  else if (df < B) df = B;
  else if (df < C) df = C;
  else df = D;

  E = fwidth(sf);

  if (sf > 0.5 - E && sf < 0.5 + E)
  {
      sf = smoothstep(0.5 - E, 0.5 + E, sf);
  }
  else
  {
      sf = step(0.5, sf);
  }


  vec3 diffuse = df * matParams.diffuse_color;
  vec3 specular = sf * matParams.specular_color;

  color = vec4(matParams.ambient_color + lightParams.light_intensity * attenuation * (diffuse + specular), 1.0);
}
