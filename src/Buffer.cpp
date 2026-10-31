#include "Buffer.hpp"
using namespace AG_EngineV2;

uint32_t Buffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,vk::PhysicalDevice* physicalDevice) {
    vk::PhysicalDeviceMemoryProperties memProperties;
    physicalDevice->getMemoryProperties(&memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

Buffer::Buffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.setSize(size)
                    .setUsage(usage)
                    .setSharingMode(vk::SharingMode::eExclusive);

    if (device.createBuffer(&bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create buffer!");
    }

    vk::MemoryRequirements memoryRequirements;
    device.getBufferMemoryRequirements(buffer, &memoryRequirements);

    vk::MemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.setAllocationSize(memoryRequirements.size)
                      .setMemoryTypeIndex(findMemoryType(memoryRequirements.memoryTypeBits, properties, &physicalDevice));

    if (device.allocateMemory(&memoryAllocateInfo, nullptr, &memory) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    device.bindBufferMemory(buffer,memory,0);
}

void Buffer::destroy(vk::Device device) {
    device.destroyBuffer(buffer);
    device.freeMemory(memory);
}