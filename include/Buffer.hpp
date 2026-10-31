#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2 {
    class Buffer {
    public:
        Buffer(){};
        Buffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,vk::PhysicalDevice* physicalDevice);
        void destroy(vk::Device device);
    };

}
