#include "Swapchain.hpp"

#include "Surface.hpp"

using namespace AG_EngineV2::Core;

Swapchain::Swapchain(vk::Device device, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice, vk::SurfaceCapabilitiesKHR surfaceCapabilities) {

    std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

    for (auto& candidate : surfaceFormats) {
        if (candidate.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = candidate;
            break;
        }
    }

    std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    for (auto& candidate : presentModes) {
        if (candidate == vk::PresentModeKHR::eMailbox) {
            presentMode = candidate;
            break;
        }
    }

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo .setSurface(surface)
                        .setMinImageCount(surfaceCapabilities.minImageCount)
                        .setImageFormat(surfaceFormat.format)
                        .setImageColorSpace(surfaceFormat.colorSpace)
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
    if (swapchainImages.empty()) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    swapchainSemaphore.resize(swapchainImages.size());

    for (auto& semaphore : swapchainSemaphore) {
        semaphore = device.createSemaphore(semaphoreCreateInfo, nullptr);
    }
}

void Swapchain::destroy(vk::Device device) {
    for (auto& semaphore : swapchainSemaphore) {
        device.destroySemaphore(semaphore);
    }
    device.destroySwapchainKHR(swapchain);
}
