#version 410 core

layout (location = 0) out vec4 color;

in VS_OUT
{
  vec3 N;
  vec3 position_view;
} fs_in;

uniform vec3 ambient_color = vec3(0.0, 0.0, 0.0);
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);

// Light pos in view space
// This shader only supports directional light, no distance fall-off
uniform vec4 light_pos;

struct AttenuationParams
{
  float Constant;
  float Linear;
  float Exp;
};

uniform AttenuationParams attenuationParams;

void main(void)
{
  vec3 lightDirection = vec3(0,0,0);
  float attenuation = 1.0f;

  if(light_pos.w == 0)
  {
    lightDirection = -light_pos.xyz;
  }
  else
  {
    lightDirection = fs_in.position_view - light_pos.xyz;
    float d = length(lightDirection);
    attenuation = attenuationParams.Constant
                  + attenuationParams.Linear * d
                  + attenuationParams.Exp * d * d;
  }

  // normalize all the incoming interpolate values
  vec3 N = normalize(fs_in.N);
  vec3 L = normalize(lightDirection);

  //...V is not really needed for lambert!
  //vec3 V = normalize(fs_in.V);

  // Nor the half-vector, needed for Blinn-Phong and the microfacet model
  //vec3 H = normalize(L + V);

  // Diffuse (or Lambertian) contribution
  // Use max, to black out faces facing away from the light. we don't really want to lit them!!
  float diffuse = clamp(dot(N,L), 0.0, 1.0);

  color = vec4(ambient_color + attenuation * diffuse * diffuse_albedo, 1.0);
}
