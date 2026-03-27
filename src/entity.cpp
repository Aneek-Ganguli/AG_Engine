#include <iostream>
#include <ostream>

#include "Entity.hpp"

Entity::Entity(Window *window, std::vector<Vertex> vertices):vertices(vertices) {
    //buffer Size
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    //Staging Buffer
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
        | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory,window);

    void* data;
    if (window->getDevice()->mapMemory(stagingBufferMemory , 0, bufferSize,{},&data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map vertex buffer memory!") ;
    }
    memcpy(data, vertices.data(), (size_t) bufferSize);
    window->getDevice()->unmapMemory(stagingBufferMemory);



    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
    vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory,window);
    copyBuffer(stagingBuffer,vertexBuffer,bufferSize,window);


    window->getDevice()->destroyBuffer(stagingBuffer,nullptr);
    window->getDevice()->freeMemory(stagingBufferMemory,nullptr);
}

void Entity::cleanUp(Window *window) {
    window->getDevice()->waitIdle();
    window->getDevice()->destroyBuffer(vertexBuffer);
    window->getDevice()->freeMemory(vertexBufferMemory);
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

void Entity::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer,vk::DeviceMemory& bufferMemory,Window* window) {

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    vertexCount = vertices.size();

    if (window->getDevice()->createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    window->getDevice()->getBufferMemoryRequirements( buffer, &memRequirements);
    window->getPhysicalDevice()->getMemoryProperties(&memProperties);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
        properties);

    if (window->getDevice()->allocateMemory( &allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    window->getDevice()->bindBufferMemory(buffer, bufferMemory, 0);
}

void Entity::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size,Window* window) {
    vk::CommandPool copyCommandPool = window->createCommandPool();
    std::vector<vk::CommandBuffer> copyCommandBuffer = window->createCommandBuffer(copyCommandPool);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    copyCommandBuffer[0].begin(beginInfo);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    copyCommandBuffer[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    copyCommandBuffer[0].end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer[0];

    if(window->getGraphicsQueue()->submit(1, &submitInfo, VK_NULL_HANDLE)!=vk::Result::eSuccess){
        throw std::runtime_error("failed to submit command buffer!");
    }
    window->getGraphicsQueue()->waitIdle();


    window->getDevice()->resetCommandPool(copyCommandPool,{});
    window->getDevice()->freeCommandBuffers(copyCommandPool,1,copyCommandBuffer.data());
    window->getDevice()->destroyCommandPool(copyCommandPool);
}
