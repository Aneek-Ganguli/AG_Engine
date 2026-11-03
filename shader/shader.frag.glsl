#version 460

layout(set = 0, binding = 0) uniform ColourBlock {
    vec4 color;
    vec2 uv;
} colourBlock;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = colourBlock.color;
}