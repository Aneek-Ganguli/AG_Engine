#include "VertexBuffer.hpp"
#include <glm/glm.hpp>
#include "core/Queue.hpp"
#include "Buffer.hpp"
#include "core/CommandBuffer.hpp"
#include "core/CommandPool.hpp"

using namespace AG_EngineV2;
using namespace AG_EngineV2::Core;

void AG_EngineV2::copyBuffer(vk::Buffer srcBuffer,vk::Buffer dstBuffer, vk::DeviceSize size, vk::Device device,Queue* queue) {
    CommandPool copyCommandPool(device,queue->graphicsQueueFamilyIndex);
    CommandBuffer copyCommandBuffer(device,copyCommandPool.commandPool);

    copyCommandBuffer.begin();
    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;

    copyCommandBuffer.commandBuffer[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    copyCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer.commandBuffer[0];

    if(queue->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit copy command buffer");
    }

    queue->graphicsQueue.waitIdle();

    device.resetCommandPool(copyCommandPool.commandPool, {});
    device.freeCommandBuffers(copyCommandPool.commandPool, 1, &copyCommandBuffer.commandBuffer[0]);
    device.destroyCommandPool(copyCommandPool.commandPool);
}

AG_EngineV2::VertexBuffer::VertexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Queue* queue, const std::vector<VertexData> vertices) {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

     stagingBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
    | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data;
    if (device.mapMemory(stagingBuffer.memory, 0, bufferSize,{},&data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map vertex buffer memory!") ;
    }
    memcpy(data, vertices.data(), (size_t) bufferSize);
    device.unmapMemory(stagingBuffer.memory);


     vertexBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    copyBuffer(stagingBuffer.buffer,vertexBuffer.buffer,bufferSize,device,queue);

    stagingBuffer.destroy(device);

    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(vertices[0]);
    bindingDesc.inputRate = vk::VertexInputRate::eVertex;
    bindingDesc.divisor = 1;

    attribDesc[0].location = 0;
    attribDesc[0].binding = 0;
    attribDesc[0].format = vk::Format::eR32G32B32Sfloat;
    attribDesc[0].offset = offsetof(VertexData,vertices);

    attribDesc[1].location = 1;
    attribDesc[1].binding = 0;
    attribDesc[1].format = vk::Format::eR32G32B32Sfloat;
    attribDesc[1].offset = offsetof(VertexData,color);

    attribDesc[2].location = 2;
    attribDesc[2].binding = 0;
    attribDesc[2].format = vk::Format::eR32G32Sfloat;
    attribDesc[2].offset = offsetof(VertexData,texCoord) ;
}

void VertexBuffer::destroy(vk::Device device) {
    vertexBuffer.destroy(device);
}
