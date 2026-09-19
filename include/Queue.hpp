#pragma once
#include <vulkan/vulkan.hpp>

#include "Device.hpp"

namespace AG_EngineV2:: Core {
    class Queue {
    public:
        Queue(){};
        Queue(vk::PhysicalDevice* physicalDevice, Device* device);
        void submitQueue(vk::Semaphore *acquireSemaphore, vk::PipelineStageFlags pipelineStages, vk::Semaphore *releaseSemaphore,vk::Fence fence,vk::CommandBuffer commandBuffer);
        void presentQueue(vk::Semaphore *releaseSemaphore, vk::SwapchainKHR *swapchain, uint32_t* imageIndex);
        uint32_t graphicsQueueFamilyIndex{0};
    private:
        vk::PresentInfoKHR presentInfo{};
        vk::Queue graphicsQueue;
    };
}

