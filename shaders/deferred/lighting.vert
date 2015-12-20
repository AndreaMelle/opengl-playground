#version 410 core

void main(void)
{
    // This constant data represents a quad in screen space
    // Will be generated only once per draw call
    const vec4 verts[4] = vec4[4](vec4(-1.0, -1.0, 0.5, 1.0),
                                  vec4( 1.0, -1.0, 0.5, 1.0),
                                  vec4(-1.0,  1.0, 0.5, 1.0),
                                  vec4( 1.0,  1.0, 0.5, 1.0));

    // We will call 4 empty vertices, getting the correct vertex from above
    // given the vertex id
    gl_Position = verts[gl_VertexID];
}
