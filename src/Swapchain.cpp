#include "Swapchain.hpp"

#include <iostream>
#include <ostream>

#include "Surface.hpp"

using namespace AG_EngineV2::Core;

Swapchain::Swapchain(vk::Device device, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice, vk::SurfaceCapabilitiesKHR surfaceCapabilities,int NumOfFramesInFlight) {

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

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.setFormat(surfaceFormat.format)
                       .setViewType(vk::ImageViewType::e2D)
                       .setSubresourceRange(vk::ImageSubresourceRange(
                           vk::ImageAspectFlagBits::eColor,
                            0,
                            1,
                            0,
                            1
                           )
                       );
    for (auto& image : swapchainImages) {
        imageViewCreateInfo.setImage(image);
        vk::ImageView imageView = device.createImageView(imageViewCreateInfo, nullptr);
        if (imageView == nullptr) {
            throw std::runtime_error("Failed to create image view!");
        }
        imageViews.push_back(imageView);
    }

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    swapchainSemaphore.resize(swapchainImages.size());

    for (auto& semaphore : swapchainSemaphore) {
        semaphore = device.createSemaphore(semaphoreCreateInfo, nullptr);
    }

    acquireSemaphore.resize(NumOfFramesInFlight);
    releaseSemaphore.resize(NumOfFramesInFlight);
    for (int i = 0; i < NumOfFramesInFlight; i++) {
        acquireSemaphore[i] = device.createSemaphore(semaphoreCreateInfo);
    }

}

uint32_t Swapchain::acquireImages(vk::Device device,int currentFrame) {
    auto acquire = device.acquireNextImageKHR(
        swapchain,
        std::numeric_limits<uint64_t>::max(),
        acquireSemaphore[currentFrame],
        VK_NULL_HANDLE
    );


    if (acquire.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to new frame");
    }

    releaseSemaphore[currentFrame] = swapchainSemaphore[acquire.value];

    return acquire.value;
}


void Swapchain::destroy(vk::Device device, int numOfFramesInFlight) {
    for (auto& semaphore : swapchainSemaphore) {
        device.destroySemaphore(semaphore);
    }
    for (auto& imageView : imageViews) {
        device.destroyImageView(imageView);
    }
    imageViews.clear();
    for (int i = 0; i < numOfFramesInFlight; i++) {
        device.destroySemaphore(acquireSemaphore[i]);
    }
    device.destroySwapchainKHR(swapchain);
}

void Swapchain::transitionImageLayoutToColour(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    transitionToColourImageBarrier.setImage(swapchainImages[imageIndex])
                                  .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                                  .setOldLayout(vk::ImageLayout::eUndefined)
                                  .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                                  .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
                                  .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                  .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);


}

void Swapchain::transitionImageLayoutToPresent(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    transitionToPresentImageBarrier.setImage(swapchainImages[imageIndex])
                                        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                                        .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                        .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                                        .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                        .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                        .setDstStageMask(vk::PipelineStageFlagBits2::eNone)
                                        .setDstAccessMask(vk::AccessFlagBits2::eNone);
}