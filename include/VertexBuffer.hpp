#pragma once
#include <vulkan/vulkan.hpp>
#include "Buffer.hpp"
#include "core/Queue.hpp"
#include <glm/glm.hpp>

namespace AG_EngineV2 {
    class VertexBuffer {
    public:
        VertexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Core::Queue* queue, std::vector<glm::vec3> vertices);
        Buffer vertexBuffer;
        Buffer stagingBuffer;
        vk::VertexInputAttributeDescription2EXT attribDesc{};
        vk::VertexInputBindingDescription2EXT bindingDesc{};
        void destroy(vk::Device device);
    };
}
