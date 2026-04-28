#include <iostream>

#include "DepthBuffer.hpp"
#include "Texture.hpp"

DepthBuffer::DepthBuffer(vk::Device* device,vk::PhysicalDevice* physicalDevice,int width,int height) {
    vk::Format depthFormat = findDepthFormat(physicalDevice);
    // std::cout << width << " " << height << "\n";
    Texture::createImage(width, height, depthFormat, vk::ImageTiling::eOptimal,
    vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
    depthImage, depthImageMemory,device,physicalDevice);
    Texture::createImageView(device,depthImage,depthImageView, depthFormat,vk::ImageAspectFlagBits::eDepth);
}
vk::Format findDepthFormat(vk::PhysicalDevice* physicalDevice) {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment,
        physicalDevice
    );
}

bool hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32Sfloat || format == vk::Format::eD32SfloatS8Uint;
}

vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features,vk::PhysicalDevice* physicalDevice) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice->getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
    
}

void DepthBuffer::cleanUp(vk::Device* device) {
    device->destroyImageView(depthImageView);
    device->freeMemory(depthImageMemory);
    device->destroyImage(depthImage);
}
