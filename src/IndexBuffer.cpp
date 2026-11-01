#include "IndexBuffer.hpp"

#include "VertexBuffer.hpp"

using namespace AG_EngineV2;
using namespace AG_EngineV2::Core;

IndexBuffer::IndexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Core::Queue* queue, const std::vector<uint16_t> &indices) {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    stagingBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
        | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data;
    if (device.mapMemory(stagingBuffer.memory, 0, bufferSize,{},&data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map vertex buffer memory!") ;
    }
    memcpy(data, indices.data(), (size_t) bufferSize);
    device.unmapMemory(stagingBuffer.memory);


    indexBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    copyBuffer(stagingBuffer.buffer,indexBuffer.buffer,bufferSize,device,queue);
    stagingBuffer.destroy(device);
}

void IndexBuffer::destroy(vk::Device device) {
    indexBuffer.destroy(device);
}
