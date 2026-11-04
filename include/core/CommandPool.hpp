#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2::Core {
    class CommandPool {
    public:
        CommandPool(){};
        CommandPool(vk::Device device, uint32_t queueFamilyIndex);
        void destroy(vk::Device device);
        vk::CommandPool commandPool;
        void resetCommandPool(vk::Device device);


    };
}