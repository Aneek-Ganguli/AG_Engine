#include "CommandBuffer.hpp"

#include "Window.hpp"

using namespace AG_EngineV2::Core;

CommandBuffer::CommandBuffer(vk::Device device, vk::CommandPool commandPool) {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setCommandPool(commandPool)
                             .setLevel(vk::CommandBufferLevel::ePrimary)
                             .setCommandBufferCount(1);

    commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo);

    if (commandBuffer.empty()) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void CommandBuffer::begin() {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    if (commandBuffer.data()->begin(&beginInfo)!= vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin command buffer!");
    }
}

void CommandBuffer::beginRendering(vk::ImageView imageView, int width, int height, vk::ClearValue clearValue) {
    vk::RenderingAttachmentInfo  colorAttachmentInfo{};
    colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentInfo.imageView = imageView;
    colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentInfo.clearValue = clearValue;

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea = vk::Rect2D(
        {0,0},
        vk::Extent2D(width, height)
    );
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount =1;
    renderingInfo.pColorAttachments = &colorAttachmentInfo;

    commandBuffer.data()->beginRendering(&renderingInfo);
}

void CommandBuffer::endRendering() {
    commandBuffer.data()->endRendering();
}

void CommandBuffer::end() {
    commandBuffer.data()->end();
}