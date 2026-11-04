#pragma once

#include <vulkan/vulkan.hpp>
#include "core/Queue.hpp"

namespace AG_EngineV2 {

    class DepthBuffer {
    public:
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;
        vk::Format format;

        DepthBuffer() = default;
        DepthBuffer(vk::Device device,Core::Queue* queue, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height);

        // Finds a supported depth format on the physical device
        static vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice);
        vk::RenderingAttachmentInfo depthAttachmentInfo{};
        // Helper to transition depth image layout for dynamic rendering
        static void transitionLayout(
            vk::Device device,
            Core::Queue* queue,
            vk::Image image,
            vk::ImageLayout oldLayout,
            vk::ImageLayout newLayout
        );

        // Clean up Vulkan depth resources
        void destroy(vk::Device device);
    };

} // namespace AG_EngineV2