#pragma once
#include "DepthBuffer.hpp"
#include "Math.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "UBO.hpp"


void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
                  vk::DeviceMemory& bufferMemory,Window* window);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,vk::PhysicalDevice* physicalDevice);

class Entity {
public:
    Entity(Window* window,std::vector<Vertex> vertices,std::vector<uint16_t> indices,Texture* p_texture);
    Entity(Model model,std::vector<ImageData> imageData,Texture* p_texture,Window* window);
    Entity() : texture() {}

    void cleanUp(Window* window);

    void draw(Window* window);
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;


    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    void updateUniformBuffer(UBO* newUBO,size_t uboSize, uint32_t currentImage);

private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    vk::PhysicalDeviceMemoryProperties memProperties;
    vk::MemoryRequirements memRequirements;
    uint32_t indexCount;
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size,Window* window);

    void createVertexBuffer(Window* window);

    void createIndexBuffer(Window* window);

    void createUniformBuffers(Window* window);


    vk::DescriptorPool descriptorPool;
    void createDescriptorPool(Window* window);

    std::vector<vk::DescriptorSet> descriptorSets;
    void createDescriptorSets(Window* window);

    Texture* texture;
    // DepthBuffer depthBuffer;
};
