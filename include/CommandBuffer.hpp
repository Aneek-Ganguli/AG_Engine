#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2::Core {
    class CommandBuffer {
    public:
        CommandBuffer(vk::Device device, vk::CommandPool commandPool);
        CommandBuffer(){};
        void begin();
        void end();
        std::vector<vk::CommandBuffer> commandBuffer;
        void beginRendering(vk::ImageView imageView, int width, int height, vk::ClearValue clearValue);
        void endRendering();
        void pipelineBarrier(int barrierCount,vk::ImageMemoryBarrier2 imageMemoryBarriers);
    };
}