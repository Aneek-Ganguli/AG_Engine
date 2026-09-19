#pragma once
#include <vulkan/vulkan.hpp>

#include "Device.hpp"

namespace AG_EngineV2:: Core {
    class Queue {
    public:
        Queue(){};
        Queue(vk::PhysicalDevice* physicalDevice, Device* device);
        void submitQueue(vk::Semaphore *acquireSemaphore, vk::PipelineStageFlags pipelineStages, vk::Semaphore *releaseSemaphore,vk::Fence fence);
        void presentQueue(vk::Semaphore *releaseSemaphore, vk::SwapchainKHR *swapchain, uint32_t* imageIndex);
    private:
        vk::PresentInfoKHR presentInfo{};
        vk::Queue graphicsQueue;
        uint32_t graphicsQueueFamilyIndex{0};
    };
}

