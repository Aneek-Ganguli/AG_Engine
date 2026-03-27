#pragma once
#include "Math.hpp"
#include "Window.hpp"

class Entity {
public:
    Entity(Window* window,std::vector<Vertex> vertices,std::vector<uint16_t> indices);
    Entity(){}
    void cleanUp(Window* window);

    void draw(Window* window);

    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;


private:
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    vk::PhysicalDeviceMemoryProperties memProperties;
    vk::MemoryRequirements memRequirements;
    uint32_t indexCount;
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory,Window* window);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size,Window* window);

    void createVertexBuffer(Window* window);

    void createIndexBuffer(Window* window);
};
