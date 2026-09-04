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

        void destroy(vk::Device device);
    };
}
