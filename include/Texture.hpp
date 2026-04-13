#pragma once
#include <stb/stb_image.h>
#include <vulkan/vulkan.hpp>

#include "Window.hpp"

class Texture {
public:
    Texture(const char* texturePath,Window* window);
    Texture() = default;

    void cleanUp(Window* window);
// private:
    stbi_uc* pixels{};
    vk::DeviceSize size;
    int width, height,textureChannels;
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;

    void createImage(uint32_t width, uint32_t height, vk::Format format,
        vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
        vk::DeviceMemory& imageMemory,Window* window);

    void transitionImageLayout(vk::Image image, vk::Format format,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                           Window* window);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image,
                           uint32_t width, uint32_t height, Window* window);

    vk::ImageView  textureImageView;
    vk::Sampler    textureSampler;

    void createTextureImageView(Window* window);
    void createTextureSampler(Window* window);
};
