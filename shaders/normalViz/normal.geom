#version 410 core

layout(triangles) in;
layout(line_strip, max_vertices=8) out;

uniform float normal_length = 0.25;
uniform mat4 MVP;
uniform int show_face_normals = 0;

in VS_OUT
{
  vec4 normal;
  vec4 color;
} geom_in[];

out vec4 vertex_color;

void main(void)
{
  // Draw the vertex normals
  int i = 0;
  for(i = 0; i < gl_in.length(); i++)
  {
    vec3 P = gl_in[i].gl_Position.xyz;
    vec3 N = geom_in[i].normal.xyz;

    gl_Position = MVP * vec4(P, 1.0f);
    vertex_color = geom_in[i].color;
    EmitVertex();

    gl_Position = MVP * vec4(P + N * normal_length, 1.0);
    vertex_color = geom_in[i].color;
    EmitVertex();

    EndPrimitive();

  }

  //Draw the face normal
  if(show_face_normals == 1)
  {
    vec3 P0 = gl_in[0].gl_Position.xyz;
    vec3 P1 = gl_in[1].gl_Position.xyz;
    vec3 P2 = gl_in[2].gl_Position.xyz;

    vec3 V0 = P0 - P1;
    vec3 V1 = P2 - P1;

    vec3 N = cross(V1, V0);
    N = normalize(N);

    // Center of the triangle
    vec3 P = (P0+P1+P2) / 3.0;

    gl_Position = MVP * vec4(P, 1.0);
    vertex_color = vec4(1, 0, 0, 1);
    EmitVertex();

    gl_Position = MVP * vec4(P + N * normal_length, 1.0);
    vertex_color = vec4(1, 0, 0, 1);
    EmitVertex();
    EndPrimitive();
  }
}
