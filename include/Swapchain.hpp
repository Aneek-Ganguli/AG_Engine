#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2::Core {

    static vk::PresentModeKHR presentMode;
    class Swapchain {
    public:
        Swapchain(vk::Device device,vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice,vk::SurfaceCapabilitiesKHR surfaceCapabilities);
        vk::SwapchainKHR swapchain{};
        std::vector<vk::Image> swapchainImages{};
        std::vector<vk::Semaphore> swapchainSemaphore{};
        void transitionImageLayoutToColour(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
        void transitionImageLayoutToPresent(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

        void destroy(vk::Device device);
        uint32_t acquireImages(vk::Device device);

        vk::Semaphore acquireSemaphore{};
        vk::Semaphore releaseSemaphore{};
        std::vector<vk::ImageView> imageViews;
        vk::ImageMemoryBarrier2 transitionToColourImageBarrier{};
        vk::ImageMemoryBarrier2 transitionToPresentImageBarrier{};
    };
}
