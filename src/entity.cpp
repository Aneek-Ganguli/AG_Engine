#include <chrono>
#include <iostream>
#include <ostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.hpp"
#include "UBO.hpp"
#include "Texture.hpp"

#define DEVICE window->getDevice()
Entity::Entity(Window* window,std::vector<Vertex> vertices,std::vector<uint16_t> indices,Texture* p_texture):vertices(vertices) , indices(indices),texture(p_texture){
    //buffer Size
    indexCount = indices.size();
    createVertexBuffer(window);
    createIndexBuffer(window);
    createUniformBuffers(window);
    createDescriptorPool(window);
    createDescriptorSets(window);
}

void Entity::cleanUp(Window *window) {
    window->getDevice()->waitIdle();

    for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        window->getDevice()->destroyBuffer(uniformBuffers[i]);
        window->getDevice()->freeMemory(uniformBuffersMemory[i]);
    }

    DEVICE->destroyDescriptorPool(descriptorPool);

    window->getDevice()->destroyBuffer(vertexBuffer);
    window->getDevice()->freeMemory(vertexBufferMemory);

    window->getDevice()->destroyBuffer(indexBuffer);
    window->getDevice()->freeMemory(indexBufferMemory);
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,Window* window) {
    vk::PhysicalDeviceMemoryProperties memProperties;
    window->getPhysicalDevice()->getMemoryProperties(&memProperties);
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

    window->getCurrentFrameData()->commandBuffer.bindIndexBuffer(indexBuffer,0,vk::IndexType::eUint16);

    window->getCurrentFrameData()->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
        window->getPipelineLayout(), 0, 1, &descriptorSets[frameIndex], 0, nullptr);
    window->getCurrentFrameData()->commandBuffer.drawIndexed(indexCount, 1,0, 0, 0);
}

void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer,vk::DeviceMemory& bufferMemory,Window* window) {

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;



    if (window->getDevice()->createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    vk::PhysicalDeviceMemoryProperties memProperties;
    vk::MemoryRequirements memRequirements;
    window->getPhysicalDevice()->getMemoryProperties(&memProperties);
    window->getDevice()->getBufferMemoryRequirements( buffer, &memRequirements);
    window->getPhysicalDevice()->getMemoryProperties(&memProperties);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
        properties,window);

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

void Entity::createVertexBuffer(Window* window) {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    //Staging Buffer
    vk::Buffer stagingBuffer{};
    vk::DeviceMemory stagingBufferMemory{};
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

void Entity::createIndexBuffer(Window* window) {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
        | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory,window);

    void* data;
    if (window->getDevice()->mapMemory(stagingBufferMemory , 0, bufferSize,{},&data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map vertex buffer memory!") ;
    }

    memcpy(data, indices.data(), (size_t) bufferSize);
    window->getDevice()->unmapMemory(stagingBufferMemory);



    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
    vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory,window);

    copyBuffer(stagingBuffer,indexBuffer,bufferSize,window);


    window->getDevice()->destroyBuffer(stagingBuffer,nullptr);
    window->getDevice()->freeMemory(stagingBufferMemory,nullptr);
}

void Entity::createUniformBuffers(Window* window) {
    vk::DeviceSize  bufferSize = sizeof(UBO);

    uniformBuffers.resize(NUM_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(NUM_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(NUM_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i],window);

        if (window->getDevice()->mapMemory(uniformBuffersMemory[i],0,bufferSize,{},&uniformBuffersMapped[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to map uniform buffer memory!") ;
        }
    }
}

void Entity::updateUniformBuffer(uint32_t currentImage) {
    UBO ubo{};

    // Setting to identity matrix (no rotation, no scale, no translation)
    ubo.model = glm::mat4(1.0f);

    // Camera stays at (2,2,2) looking at the origin (0,0,0)
    // Up vector is (0,1,0) - Y-up is standard for GLM
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Perspective calc
    ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float)600, 0.1f, 10.0f);

    // Vulkan flip (GLM was designed for OpenGL where Y is up, Vulkan Y is down)
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Entity::createDescriptorPool(Window* window) {
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(NUM_FRAMES_IN_FLIGHT);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(NUM_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(NUM_FRAMES_IN_FLIGHT);

    if (DEVICE->createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Entity::createDescriptorSets(Window* window) {
    std::vector<vk::DescriptorSetLayout> layouts(NUM_FRAMES_IN_FLIGHT, window->getDescriptorSetLayout());
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(NUM_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(NUM_FRAMES_IN_FLIGHT);
    if (DEVICE->allocateDescriptorSets(&allocInfo, descriptorSets.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UBO);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = texture->textureImageView;
        imageInfo.sampler = texture->textureSampler;

        // Use vk::WriteDescriptorSet instead of the C struct
        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};

        // First descriptor: Uniform Buffer
        descriptorWrites[0]
            .setDstSet(descriptorSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setPBufferInfo(&bufferInfo);

        // Second descriptor: Combined Image Sampler
        descriptorWrites[1]
            .setDstSet(descriptorSets[i])
            .setDstBinding(1)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setPImageInfo(&imageInfo);

        // Call the device method directly
        DEVICE->updateDescriptorSets(descriptorWrites, nullptr);

        // DEVICE->updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
}