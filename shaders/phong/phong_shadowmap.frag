#version 410 core

layout (location = 0) out vec4 color;

in VS_OUT
{
  vec3 N;
  vec4 L_modelview;
  vec3 V_modelview;
  vec4 P_shadowmap;
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

struct ShadowParams
{
    sampler2DShadow shadowmap;
    float bias;
};

uniform ShadowParams shadowParams;
uniform uint mode;

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ),
   vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ),
   vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ),
   vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ),
   vec2( 0.14383161, -0.14100790 )
);

float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
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

  vec3 R = reflect(-L, N);

  vec3 diffuse = max(dot(N,L), 0.0) * matParams.diffuse_color;
  vec3 specular = pow(max(dot(R,V), 0.0), matParams.specular_power) * matParams.specular_color;

  float visibility = 1.0;
  // texture is setup for comparison. we use z in light space with the bias
  //float cosTheta = clamp( dot( N,L ), 0,1 );
  //float varBias = shadowParams.bias * tan(acos(cosTheta));
	//varBias = clamp(varBias, 0, 0.01);
  float varBias = shadowParams.bias;

  for(int i = 0; i < 4; i++)
  {
    // use either :
    //  - Always the same samples.
    //    Gives a fixed pattern in the shadow, but no noise
    //int index = i;
    //  - A random sample, based on the pixel's screen location.
    //    No banding, but the shadow moves with the camera, which looks weird.
    //int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
    //  - A random sample, based on the pixel's position in world space.
    //    The position is rounded to the millimeter to avoid too much aliasing
    int index = int(16.0 * random(floor(fs_in.V_modelview * 1000.0), i)) % 16;

    // being fully in the shadow will eat up 4*0.2 = 0.8
    // 0.2 potentially remain, which is quite dark.
    visibility -= 0.2 * (1.0 - texture( shadowParams.shadowmap,
        vec3(fs_in.P_shadowmap.xy + poissonDisk[index]/700.0,  (fs_in.P_shadowmap.z-varBias)/fs_in.P_shadowmap.w) ));
  }

  // Without Poisson sampling
  //visibility = texture( shadowParams.shadowmap, vec3(fs_in.P_shadowmap.xy, (fs_in.P_shadowmap.z - varBias)/fs_in.P_shadowmap.w) );

  if(mode == 1)
  {
      color = vec4(matParams.ambient_color
            + lightParams.light_intensity * attenuation * visibility * (diffuse + specular), 1.0);
  }
  else if(mode == 3)
  {
      color = vec4(matParams.ambient_color
            + lightParams.light_intensity * attenuation * (diffuse + specular), 1.0);
  }
  else if(mode == 4)
  {
      color = vec4(visibility, visibility, visibility, 1.0);
  }

}
