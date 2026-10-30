#version 450


void main(){
    vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Top center
    vec2(0.5, 0.5),    // Bottom right
    vec2(-0.5, 0.5)    // Bottom left
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}