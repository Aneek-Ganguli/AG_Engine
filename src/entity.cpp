#include <iostream>
#include <ostream>

#include "Entity.hpp"

Entity::Entity(Window *window, std::vector<Vertex> vertices):vertices(vertices) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    vertexCount = vertices.size();

    if (window->getDevice()->createBuffer(&bufferInfo, nullptr, &vertexBuffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    window->getDevice()->getBufferMemoryRequirements( vertexBuffer, &memRequirements);
    window->getPhysicalDevice()->getMemoryProperties(&memProperties);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    if (window->getDevice()->allocateMemory( &allocInfo, nullptr, &vertexBufferMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    window->getDevice()->bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

    void* data;
    if (window->getDevice()->mapMemory(vertexBufferMemory, 0, bufferInfo.size,
        {},&data) != vk::Result::eSuccess) {
        std::cout << "failed to map vertex buffer memory!" << std::endl;
    }
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
    window->getDevice()->unmapMemory(vertexBufferMemory);
}

void Entity::cleanUp(Window *window) {
    window->getDevice()->freeMemory(vertexBufferMemory);
    window->getDevice()->destroyBuffer(vertexBuffer);
}

uint32_t Entity::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


void Entity::draw(Window *window) {
    vk::Buffer vertexBuffers[] = {vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    window->getCurrentFrameData()->commandBuffer.bindVertexBuffers(0,1,vertexBuffers,offsets);
    window->getCurrentFrameData()->commandBuffer.draw(3, 1, 0, 0);
}