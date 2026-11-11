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
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;



void main() {
    vec3 directionalLightDirection =(vec4(ubo.lightPosition,1.0f) - (vec4(inPosition,1.0f) * ubo.model)).xyz;
    mat4 transform = ubo.proj * ubo.view * ubo.model;

    gl_Position =  transform * vec4(inPosition, 1.0);
    float attenuation = 1.0f / dot(directionalLightDirection, directionalLightDirection);
    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w  * attenuation;
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(normal * mat3(ubo.model)), normalize(directionalLightDirection)), 0.0f);


    fragColor = color * (diffuseLight+ambientLight);

    fragTexCoord = (uv - ubo.uvOffset) * ubo.uvScale;
}