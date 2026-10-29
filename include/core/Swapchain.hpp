#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2::Core {

    static vk::PresentModeKHR presentMode;
    class Swapchain {
    public:
        Swapchain(vk::Device device,vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice,vk::SurfaceCapabilitiesKHR surfaceCapabilities, int NumOfFramesInFlight); //  if no frames in flight set 0
        vk::SwapchainKHR swapchain{};
        std::vector<vk::Image> swapchainImages{};
        std::vector<vk::Semaphore> swapchainSemaphore{};
        void transitionImageLayoutToColour(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
        void transitionImageLayoutToPresent(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

        void destroy(vk::Device device, int numOfFramesInFlight = 0);
        uint32_t acquireImages(vk::Device device, int currentFrame); // if no frames in flight set 0

        std::vector<vk::Semaphore> acquireSemaphore{};
        std::vector<vk::Semaphore> releaseSemaphore{};
        std::vector<vk::ImageView> imageViews;
        vk::ImageMemoryBarrier2 transitionToColourImageBarrier{};
        vk::ImageMemoryBarrier2 transitionToPresentImageBarrier{};
    };
}
