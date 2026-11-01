#pragma once
#include <vulkan/vulkan.hpp>
#include "Buffer.hpp"

namespace  AG_EngineV2 {
    class UniformBuffer {
    public:
        UniformBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, void *uniformData, uint32_t offset);
        Buffer uniformBuffer;
        vk::DeviceSize size;
        vk::DescriptorBufferInfo bufferInfo{};
        vk::WriteDescriptorSet descriptorWrite{};
        void destroy(vk::Device device);
        void* data;
        void updateData(void* updateData);
    };
}