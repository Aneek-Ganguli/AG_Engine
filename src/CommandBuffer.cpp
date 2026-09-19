#include "CommandBuffer.hpp"

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

void CommandBuffer::end() {
    commandBuffer.data()->end();
}