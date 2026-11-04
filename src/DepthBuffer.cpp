#include "DepthBuffer.hpp"
#include "core/CommandPool.hpp"
#include "core/CommandBuffer.hpp"
#include <stdexcept>
#include <vector>

namespace AG_EngineV2 {

    // Helper to locate suitable GPU memory type index
    static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice physicalDevice) {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type for depth buffer!");
    }

    vk::Format DepthBuffer::findDepthFormat(vk::PhysicalDevice physicalDevice) {
        std::vector<vk::Format> candidates = {
            vk::Format::eD32Sfloat,
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint
        };

        for (vk::Format candidateFormat : candidates) {
            vk::FormatProperties props = physicalDevice.getFormatProperties(candidateFormat);
            if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                return candidateFormat;
            }
        }

        throw std::runtime_error("failed to find supported depth format!");
    }

    DepthBuffer::DepthBuffer(vk::Device device, Core::Queue* queue, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height) {
        format = findDepthFormat(physicalDevice);

        // 1. Create Image
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;

        if (device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create depth image!");
        }

        // 2. Allocate and bind memory
        vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, physicalDevice);

        if (device.allocateMemory(&allocInfo, nullptr, &memory) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate depth image memory!");
        }
        device.bindImageMemory(image, memory, 0);

        // 3. Create ImageView
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (device.createImageView(&viewInfo, nullptr, &view) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create depth image view!");
        }

        transitionLayout(device, queue, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }

    void DepthBuffer::transitionLayout(vk::Device device, Core::Queue* queue, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        Core::CommandPool copyCommandPool(device, queue->graphicsQueueFamilyIndex);
        Core::CommandBuffer copyCommandBuffer(device, copyCommandPool.commandPool);

        copyCommandBuffer.begin();

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        } else {
            throw std::invalid_argument("unsupported depth layout transition!");
        }

        copyCommandBuffer.commandBuffer[0].pipelineBarrier(
            sourceStage, destinationStage,
            {},
            nullptr, nullptr,
            barrier
        );

        copyCommandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &copyCommandBuffer.commandBuffer[0];

        if (queue->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to submit depth layout transition command buffer!");
        }
        queue->graphicsQueue.waitIdle();

        device.resetCommandPool(copyCommandPool.commandPool, {});
        device.freeCommandBuffers(copyCommandPool.commandPool, 1, &copyCommandBuffer.commandBuffer[0]);
        device.destroyCommandPool(copyCommandPool.commandPool);
    }

    void DepthBuffer::destroy(vk::Device device) {
        if (view) {
            device.destroyImageView(view);
            view = nullptr;
        }
        if (image) {
            device.destroyImage(image);
            image = nullptr;
        }
        if (memory) {
            device.freeMemory(memory);
            memory = nullptr;
        }
    }

} // namespace AG_EngineV2