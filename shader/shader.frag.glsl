#version 460

layout(set = 0, binding = 0) uniform ColourBlock {
    vec3 color;
} colourBlock;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(colourBlock.color, 1.0f);
}