#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

struct PerFrameData {
    vk::CommandBuffer commandBuffer{};
    vk::CommandPool commandPool{};
    vk::Semaphore acquireSemaphore{};
    vk::Fence fence{};
};