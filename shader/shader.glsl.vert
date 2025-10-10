#version 440

layout(set=1,binding=0)uniform UBO{
    mat4 mvp;
};

layout(set=1,binding=1)uniform ColorUniform{
    vec4 color;
};

layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;


void main(){
    gl_Position = mvp * vec4(position,1);
    outColor = color;
    outTexcoord = texcoord;
}