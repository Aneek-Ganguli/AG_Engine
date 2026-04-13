#pragma once
#include <vulkan/vulkan.hpp>

struct VertexInfo {
public:
    explicit VertexInfo();
    vk::VertexInputBindingDescription bindingDescription{};
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};
};