#pragma once
#include <vulkan/vulkan.hpp>

class DepthBuffer {
public:
    DepthBuffer() = default;

    DepthBuffer(vk::Device* device,vk::PhysicalDevice* physicalDevice, int width, int height);

    void cleanUp(vk::Device* device);
// private:
    vk::Image depthImage;
    vk::DeviceMemory depthImageMemory;
    vk::ImageView depthImageView;
};

vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features,vk::PhysicalDevice* physicalDevice);
vk::Format findDepthFormat(vk::PhysicalDevice* physicalDevice);
bool hasStencilComponent(vk::Format format);