#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2::Core {
    class Swapchain {
    public:
        Swapchain(vk::Device device,vk::SurfaceKHR surface,vk::SurfaceCapabilitiesKHR surfaceCapabilities);
        vk::SwapchainKHR swapchain{};
        std::vector<vk::Image> swapchainImages{};

        void destroy(vk::Device device);
    };
}
