#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(8)  glm::vec2 texCoord;
};

struct Model {
    Model()=default;
    Model(float x,float y, float extentX,float extentY,float windowWidth, float windowHeight); //un-normalized
    std::vector<glm::vec3> vertices;
    std::vector<uint16_t> indices;
};

struct ImageData {
    alignas(16) glm::vec3 color;
    alignas(8)  glm::vec2 texCoord;
};

void normalize(float &x, float &y, float &z,float windowWidth, float windowHeight);

std::vector<ImageData> solidColor(Model* model, glm::vec3 color);