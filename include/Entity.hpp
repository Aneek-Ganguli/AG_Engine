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


private:
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    std::vector<Vertex> vertices;
    vk::PhysicalDeviceMemoryProperties memProperties;
    vk::MemoryRequirements memRequirements;
    vk::DeviceMemory vertexBufferMemory;
    uint32_t vertexCount;
};
