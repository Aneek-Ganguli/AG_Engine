#pragma once
#include <vulkan/vulkan.hpp>

#include "Device.hpp"

namespace AG_EngineV2:: Core {
    class Queue {
    public:
        Queue(){};
        Queue(vk::PhysicalDevice* physicalDevice, Device* device);
    private:
        vk::Queue graphicsQueue;
        uint32_t graphicsQueueFamilyIndex{0};
    };
}

