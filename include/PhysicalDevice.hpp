#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2 {
    namespace Core{
        class PhysicalDevice {
        public:
            PhysicalDevice(vk::Instance instance);
            PhysicalDevice(){};
            vk::PhysicalDevice* getPhysicalDevicePtr(){return &physicalDevice;};
            vk::PhysicalDevice physicalDevice;
        };
    }
}