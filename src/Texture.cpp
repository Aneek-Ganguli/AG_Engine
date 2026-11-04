#include "Texture.hpp"
#include "core/Queue.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Buffer.hpp"
#include "../lib/stb/stb_image.h"
#include "core/CommandBuffer.hpp"
#include "core/CommandPool.hpp"

using namespace AG_EngineV2;
using namespace AG_EngineV2::Core;

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,vk::PhysicalDevice* physicalDevice) {
    vk::PhysicalDeviceMemoryProperties memProperties;
    physicalDevice->getMemoryProperties(&memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

Texture::Texture(vk::Device device,Queue* queue, vk::PhysicalDevice physicalDevice,std::string texturePath, bool flipVertical) {
    createImageTexture(device,queue, physicalDevice, texturePath, flipVertical);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = textureImage;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eR8G8B8A8Srgb;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (device.createImageView(&viewInfo, nullptr, &view) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture image view!");
    }

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eNearest;
    samplerInfo.minFilter = vk::Filter::eNearest;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    if (device.createSampler(&samplerInfo, nullptr, &sampler) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture sampler!");
    }


    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = view;
    imageInfo.sampler = sampler;


    textureWrite.dstBinding = 1; // Binding index inside Set 1
    textureWrite.dstArrayElement = 0;
    textureWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    textureWrite.descriptorCount = 1;
    textureWrite.pImageInfo = &imageInfo;

}

void Texture::transitionImageLayout(vk::Device device, Core::Queue* queue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
    CommandPool copyCommandPool(device, queue->graphicsQueueFamilyIndex);
    CommandBuffer copyCommandBuffer(device, copyCommandPool.commandPool);

    copyCommandBuffer.begin();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels; // FIXED: Cover all mip levels!
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    copyCommandBuffer.commandBuffer[0].pipelineBarrier(
        sourceStage, destinationStage,
        {},
        nullptr, nullptr,
        barrier
    );

    copyCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer.commandBuffer[0];

    if (queue->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit layout transition command buffer!");
    }
    queue->graphicsQueue.waitIdle();

    device.resetCommandPool(copyCommandPool.commandPool, {});
    device.freeCommandBuffers(copyCommandPool.commandPool, 1, &copyCommandBuffer.commandBuffer[0]);
    device.destroyCommandPool(copyCommandPool.commandPool);
}

void Texture::createImageTexture(vk::Device device,Queue* queue,vk::PhysicalDevice physicalDevice,const std::string& texturePath, bool flipVertical) {
stbi_set_flip_vertically_on_load(flipVertical);
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    // 1. Create the Image first
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vk::Format::eR8G8B8A8Srgb;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.flags = {};

    if (device.createImage(&imageInfo, nullptr, &textureImage) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create image!");
    }

    // 2. Allocate and bind dedicated device memory for the image
    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(textureImage);
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, &physicalDevice);

     // Store this in your class member if you need to free it later
    if (device.allocateMemory(&allocInfo, nullptr, &textureImageMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    device.bindImageMemory(textureImage, textureImageMemory, 0);

    // 3. Create Staging Buffer and copy pixel data
    stagingBuffer = Buffer(
        device,
        physicalDevice,
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void* mem = nullptr;
    if (device.mapMemory(stagingBuffer.memory, 0, imageSize, {}, &mem) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map memory!");
    }
    memcpy(mem, pixels, static_cast<size_t>(imageSize));
    device.unmapMemory(stagingBuffer.memory);

    transitionImageLayout(device,queue,textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,mipLevels);


    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};

    CommandPool copyCommandPool(device,queue->graphicsQueueFamilyIndex);
    CommandBuffer copyCommandBuffer(device,copyCommandPool.commandPool);
    copyCommandBuffer.begin();
    copyCommandBuffer.commandBuffer[0].copyBufferToImage(
            stagingBuffer.buffer,
            textureImage,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &region
    );


    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Srgb);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    // copyCommandBuffer.begin();

    vk::ImageMemoryBarrier barrier;
    barrier.image = textureImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        copyCommandBuffer.commandBuffer[0].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            nullptr,
            nullptr,
            barrier
        );

        vk::ImageBlit blit;
        blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D{
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        // commandBuffer.blitImage(
        //     image, vk::ImageLayout::eTransferSrcOptimal,
        //     image, vk::ImageLayout::eTransferDstOptimal,
        //     blit,
        //     vk::Filter::eLinear
        // );

        copyCommandBuffer.commandBuffer[0].blitImage(
            textureImage, vk::ImageLayout::eTransferSrcOptimal,
            textureImage, vk::ImageLayout::eTransferDstOptimal,
            1,
            &blit,
            vk::Filter::eLinear
        );

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        copyCommandBuffer.commandBuffer[0].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            nullptr,
            nullptr,
            barrier
        );

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

// Extra transition for the last mip level since it isn't handled inside the loop loop's blit pass
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    copyCommandBuffer.commandBuffer[0].pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {},
        nullptr,
        nullptr,
        barrier
    );

    // TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    stbi_image_free(pixels);

    copyCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer.commandBuffer[0];

    if (queue->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit texture command buffer!");
    }
    queue->graphicsQueue.waitIdle();

    device.resetCommandPool(copyCommandPool.commandPool, {});
    device.freeCommandBuffers(copyCommandPool.commandPool, 1, &copyCommandBuffer.commandBuffer[0]);
    device.destroyCommandPool(copyCommandPool.commandPool);
}

void Texture::destroy(vk::Device device) {
    device.freeMemory(textureImageMemory);
    stagingBuffer.destroy(device);
    device.destroyImageView(view);
    device.destroySampler(sampler);
    device.destroyImage(textureImage);
}