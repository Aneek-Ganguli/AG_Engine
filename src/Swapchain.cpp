#include "Swapchain.hpp"

#include <iostream>
#include <ostream>

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

    uint32_t imageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);
    }


    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo .setSurface(surface)
                        .setMinImageCount(imageCount)
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

    acquireSemaphore = device.createSemaphore(semaphoreCreateInfo);

}

void Swapchain::acquireImages(vk::Device device) {
    auto acquire = device.acquireNextImageKHR(
        swapchain,
        std::numeric_limits<uint64_t>::max(),
        acquireSemaphore,
        VK_NULL_HANDLE
    );


    if (acquire.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to new frame");
    }

    imageIndex = acquire.value;

    releaseSemaphore = swapchainSemaphore[imageIndex];
}


void Swapchain::destroy(vk::Device device) {
    for (auto& semaphore : swapchainSemaphore) {
        device.destroySemaphore(semaphore);
    }
    device.destroySwapchainKHR(swapchain);
}
