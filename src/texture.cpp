#define STB_IMAGE_IMPLEMENTATION
#include "Texture.hpp"
#include "Entity.hpp"

#define DEVICE window->getDevice()

// ---------------------------------------------------------------------------
// Transition an image from oldLayout → newLayout using a pipeline barrier
// ---------------------------------------------------------------------------
void Texture::transitionImageLayout(vk::Image image, vk::Format /*format*/,
                                    vk::ImageLayout oldLayout,
                                    vk::ImageLayout newLayout,
                                    Window* window)
{
    // Create a short-lived command pool + buffer (same as Entity::copyBuffer)
    vk::CommandPool pool = window->createCommandPool();
    std::vector<vk::CommandBuffer> bufs = window->createCommandBuffer(pool);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    bufs[0].begin(beginInfo);

    // ---- image memory barrier ----
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout           = oldLayout;
    barrier.newLayout           = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        // No need to wait on anything before the transfer write
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        // Fragment shader must wait for the transfer write to finish
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    bufs[0].pipelineBarrier(
        srcStage, dstStage,
        {},          // dependency flags
        0, nullptr,  // memory barriers
        0, nullptr,  // buffer memory barriers
        1, &barrier  // image memory barriers
    );

    bufs[0].end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &bufs[0];

    if (window->getGraphicsQueue()->submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit transition command buffer!");
    }
    window->getGraphicsQueue()->waitIdle();

    DEVICE->freeCommandBuffers(pool, 1, bufs.data());
    DEVICE->destroyCommandPool(pool);
}

// ---------------------------------------------------------------------------
// Copy a VkBuffer into a VkImage (image must already be in TransferDstOptimal)
// ---------------------------------------------------------------------------
void Texture::copyBufferToImage(vk::Buffer buffer, vk::Image image,
                                uint32_t w, uint32_t h,
                                Window* window)
{
    vk::CommandPool pool = window->createCommandPool();
    std::vector<vk::CommandBuffer> bufs = window->createCommandBuffer(pool);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    bufs[0].begin(beginInfo);

    vk::BufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;  // tightly packed
    region.bufferImageHeight = 0;  // tightly packed

    region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{w, h, 1};

    bufs[0].copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,
        1, &region
    );

    bufs[0].end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &bufs[0];

    if (window->getGraphicsQueue()->submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit copyBufferToImage command buffer!");
    }
    window->getGraphicsQueue()->waitIdle();

    DEVICE->freeCommandBuffers(pool, 1, bufs.data());
    DEVICE->destroyCommandPool(pool);
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
Texture::Texture(const char* texturePath, Window* window) {
    // 1. Load pixels from disk
    stbi_set_flip_vertically_on_load(true);
    pixels = stbi_load(texturePath, &width, &height, &textureChannels, STBI_rgb_alpha);
    size   = width * height * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 2. Upload pixels into a host-visible staging buffer
    createBuffer(size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory, window);

    void* data;
    if (DEVICE->mapMemory(stagingBufferMemory, 0, size, {}, &data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map staging buffer memory!");
    }
    memcpy(data, pixels, static_cast<size_t>(size));
    DEVICE->unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);
    pixels = nullptr;

    // 3. Create the device-local VkImage
    createImage(width, height,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        textureImage, textureImageMemory, window);

    // 4. Transition: Undefined → TransferDstOptimal
    transitionImageLayout(textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        window);

    // 5. Copy staging buffer → image
    copyBufferToImage(stagingBuffer, textureImage,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        window);

    // 6. Transition: TransferDstOptimal → ShaderReadOnlyOptimal
    transitionImageLayout(textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        window);

    // 7. Clean up staging buffer — no longer needed
    DEVICE->destroyBuffer(stagingBuffer, nullptr);
    DEVICE->freeMemory(stagingBufferMemory, nullptr);

    // 8. Create image view so shaders can sample the texture
    createTextureImageView(window);

    // 9. Create sampler (filtering, addressing modes, anisotropy)
    createTextureSampler(window);
}

// ---------------------------------------------------------------------------
// createImage  (bug fix: use the `image`/`imageMemory` params, not members)
// ---------------------------------------------------------------------------
void Texture::createImage(uint32_t width, uint32_t height,
                          vk::Format format, vk::ImageTiling tiling,
                          vk::ImageUsageFlags usage,
                          vk::MemoryPropertyFlags properties,
                          vk::Image& image, vk::DeviceMemory& imageMemory,
                          Window* window)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType             = vk::ImageType::e2D;
    imageInfo.extent.width          = static_cast<uint32_t>(width);
    imageInfo.extent.height         = static_cast<uint32_t>(height);
    imageInfo.extent.depth          = 1;
    imageInfo.mipLevels             = 1;
    imageInfo.arrayLayers           = 1;
    imageInfo.format                = format;
    imageInfo.tiling                = tiling;
    imageInfo.initialLayout         = vk::ImageLayout::eUndefined; // <-- required
    imageInfo.usage                 = usage;
    imageInfo.samples               = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode           = vk::SharingMode::eExclusive;

    // BUG FIX: was using textureImage/textureImageMemory directly; now uses params
    if (DEVICE->createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create image!");
    }

    vk::MemoryRequirements memRequirements;
    DEVICE->getImageMemoryRequirements(image, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, window);

    if (DEVICE->allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    DEVICE->bindImageMemory(image, imageMemory, 0);
}

// ---------------------------------------------------------------------------
// createTextureImageView
// Creates a VkImageView for the texture so shaders can sample it.
// ---------------------------------------------------------------------------
void Texture::createTextureImageView(Window* window) {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image    = textureImage;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format   = vk::Format::eR8G8B8A8Srgb;

    // Which aspect of the image (color, depth, stencil…)
    viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    // components left at default (eIdentity == 0 for all channels)

    if (DEVICE->createImageView(&viewInfo, nullptr, &textureImageView) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

// ---------------------------------------------------------------------------
// createTextureSampler
// Creates a VkSampler with linear filtering and anisotropic filtering.
// ---------------------------------------------------------------------------
void Texture::createTextureSampler(Window* window) {
    // Query device limits to find the max supported anisotropy level
    vk::PhysicalDeviceProperties properties{};
    window->getPhysicalDevice()->getProperties(&properties);

    vk::SamplerCreateInfo samplerInfo{};

    // Linear interpolation for both magnification (oversampling) and
    // minification (undersampling)
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    // Repeat the texture when UV coords go beyond [0, 1)
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

    // Anisotropic filtering — use the highest quality the device supports
    samplerInfo.anisotropyEnable = vk::False;
    samplerInfo.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    // Color returned when addressMode is eClampToBorder (not used here)
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

    // Use normalised [0, 1) UV coordinates (standard for textures)
    samplerInfo.unnormalizedCoordinates = vk::False;

    // No comparison / shadow-map PCF filtering
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp     = vk::CompareOp::eAlways;

    // Mip-mapping (single mip level for now)
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = 0.0f;

    if (DEVICE->createSampler(&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

// ---------------------------------------------------------------------------
// Cleanup
// Destroy in reverse-creation order: sampler → view → image → memory
// ---------------------------------------------------------------------------
void Texture::cleanUp(Window* window) {
    DEVICE->waitIdle();
    DEVICE->destroySampler(textureSampler, nullptr);
    DEVICE->destroyImageView(textureImageView, nullptr);
    DEVICE->destroyImage(textureImage, nullptr);
    DEVICE->freeMemory(textureImageMemory, nullptr);
}