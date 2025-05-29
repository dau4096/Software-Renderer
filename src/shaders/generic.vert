/* generic.vert */
#version 460 core

void main() {
    const vec2 positions[4] = vec2[4](
        vec2(-1.0, -1.0), //Bottom Left
        vec2( 1.0, -1.0), //Bottom Right
        vec2(-1.0,  1.0), //Top Left
        vec2( 1.0,  1.0)  //Top Right
    );
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}