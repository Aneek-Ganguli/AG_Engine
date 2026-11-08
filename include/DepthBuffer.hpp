#pragma once

#include <vulkan/vulkan.hpp>
#include "core/Queue.hpp"

namespace AG_EngineV2 {

    class DepthBuffer {
    public:
        vk::Image image{};
        vk::DeviceMemory memory{};
        vk::ImageView view{};
        vk::Format format{};

        DepthBuffer() = default;
        DepthBuffer(vk::Device device,Core::Queue* queue, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height);

        static vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice);
        vk::RenderingAttachmentInfo depthAttachmentInfo{};
        static void transitionLayout(
            vk::Device device,
            Core::Queue* queue,
            vk::Image image,
            vk::ImageLayout oldLayout,
            vk::ImageLayout newLayout
        );

        void destroy(vk::Device device);
    };

} // namespace AG_EngineV2