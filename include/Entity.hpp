#pragma once
#include "Math.hpp"
#include "Window.hpp"

class Entity {
public:
    Entity(Window* window,std::vector<Vertex> vertices);
    Entity(){}
    void cleanUp(Window* window);

    void draw(Window* window);

    vk::Buffer vertexBuffer;
    vk::Buffer stagingBuffer{};
    vk::DeviceMemory stagingBufferMemory{};

private:
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    std::vector<Vertex> vertices;
    vk::PhysicalDeviceMemoryProperties memProperties;
    vk::MemoryRequirements memRequirements;
    vk::DeviceMemory vertexBufferMemory;
    uint32_t vertexCount;
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory,Window* window);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size,Window* window);
};
