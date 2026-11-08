#pragma once
#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "core/Queue.hpp"

namespace AG_EngineV2 {
    struct Texture {
        Texture()=default;
        Texture(vk::Device device,Core::Queue* queue, vk::PhysicalDevice physicalDevice, std::string texturePath, bool flipVertical);
        void createImageTexture(vk::Device device,Core::Queue *queue,vk::PhysicalDevice physicalDevice,const std::string& texturePath, bool flipVertical);
        void transitionImageLayout(vk::Device device,Core::Queue* queue,vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
        uint32_t mipLevels;
        vk::Image textureImage{};
        void* mem;
        vk::ImageView view{};
        vk::Sampler sampler{};
        void destroy(vk::Device device);
        vk::DeviceMemory textureImageMemory{};
        Buffer stagingBuffer{};
        vk::DescriptorImageInfo imageInfo{};
        vk::WriteDescriptorSet textureWrite{};
        void update(int index, vk::DescriptorSet bindlessTextureDescriptorSet, vk::Device device);
    };
}
