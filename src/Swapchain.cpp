#include "Swapchain.hpp"

#include "Surface.hpp"

using namespace AG_EngineV2::Core;

Swapchain::Swapchain(vk::Device device, vk::SurfaceKHR surface, vk::SurfaceCapabilitiesKHR surfaceCapabilities) {
    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo .setSurface(surface)
                        .setMinImageCount(surfaceCapabilities.minImageCount)
                        .setImageFormat(surfaceFormat)
                        .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                        .setImageExtent(surfaceCapabilities.currentExtent)
                        .setImageArrayLayers(1)
                        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                        .setImageSharingMode(vk::SharingMode::eExclusive)
                        .setPreTransform(surfaceCapabilities.currentTransform)
                        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                        .setPresentMode(vk::PresentModeKHR::eFifo)
                        .setClipped(true);
    if (device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    swapchainImages = device.getSwapchainImagesKHR(swapchain);
}

void Swapchain::destroy(vk::Device device) {
    device.destroySwapchainKHR(swapchain);
}
