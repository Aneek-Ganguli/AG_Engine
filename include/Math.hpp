#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(8)  glm::vec2 texCoord;
};