#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0, std140) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 uvOffset;
    vec2 uvScale;
} ubo;

layout(binding = 3) uniform ShaderUniform{
    uint idk;
} shading;

const vec3 directionalLightDirection = normalize(vec3(1.0f, -3.0f, -1.0f));


void main() {
    mat4 transform = ubo.proj * ubo.view * ubo.model;
    gl_Position =  transform * vec4(inPosition, 1.0);
    float lightIntensity = max(dot(normal, directionalLightDirection), 0.0f);
    fragColor = color * lightIntensity;
//    fragColor = color;
    fragTexCoord = (uv - ubo.uvOffset) * ubo.uvScale;
}