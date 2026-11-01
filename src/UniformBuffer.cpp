#include "UniformBuffer.hpp"

using namespace AG_EngineV2;

UniformBuffer::UniformBuffer(vk::Device device,vk::PhysicalDevice physicalDevice, vk::DeviceSize size, void *uniformData, uint32_t offset)
    :size(size) {

    uniformBuffer = Buffer(device, physicalDevice, size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent);
    // data = nullptr;
    if(device.mapMemory(uniformBuffer.memory, 0, size, {}, &data) !=  vk::Result::eSuccess) {
        throw std::runtime_error("Failed to map uniform buffer memory");
    }

    // 2. Copy the data into the mapped pointer
    memcpy(data, uniformData, size);

    // 3. Unmap the memory (or keep it mapped persistently if preferred)
    // device.unmapMemory(uniformBuffer.memory);


    bufferInfo.buffer = uniformBuffer.buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = size;


    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
}

void UniformBuffer::destroy(vk::Device device) {
    uniformBuffer.destroy(device);
}

void UniformBuffer::updateData(void *updateData) {
    memcpy(data, updateData, size);
}
