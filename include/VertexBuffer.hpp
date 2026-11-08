#pragma once
#include <vulkan/vulkan.hpp>
#include "Buffer.hpp"
#include "core/Queue.hpp"
#include <glm/glm.hpp>

namespace AG_EngineV2 {
    void copyBuffer(vk::Buffer srcBuffer,vk::Buffer dstBuffer, vk::DeviceSize size, vk::Device device, Core::Queue* queue);

    struct VertexData {
        glm::vec3 vertices;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

    class VertexBuffer {
    public:
        VertexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Core::Queue* queue, std::vector<VertexData> vertices);
        VertexBuffer() = default;
        Buffer vertexBuffer{};
        Buffer stagingBuffer{};
        std::array<vk::VertexInputAttributeDescription2EXT,3> attribDesc{};
        vk::VertexInputBindingDescription2EXT bindingDesc{};
        void destroy(vk::Device device);
    };
}
