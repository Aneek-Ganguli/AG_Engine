#pragma once
#include <vulkan/vulkan.hpp>
#include "Buffer.hpp"
#include "core/Queue.hpp"

namespace AG_EngineV2{
    class IndexBuffer {
    public:
        IndexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Core::Queue* queue, const std::vector<uint16_t> &indices);
        IndexBuffer() = default;
        Buffer indexBuffer;
        Buffer stagingBuffer;
        void destroy(vk::Device device);
    };
}
