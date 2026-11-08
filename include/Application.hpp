#pragma once
#include <string>

#include "BindlessTextures.hpp"
#include "DepthBuffer.hpp"
#include "DescriptorSet.hpp"
#include "GraphicsPipeline.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "core/CommandBuffer.hpp"
#include "core/CommandPool.hpp"
#include "core/Device.hpp"
#include "core/Fence.hpp"
#include "core/Queue.hpp"
#include "core/Surface.hpp"
#include "core/Swapchain.hpp"
#include "core/Window.hpp"

using namespace AG_EngineV2::Core;
const uint8_t constexpr NumFramesInFlight = 3;

namespace AG_EngineV2 {
    class Application {
    public:
        Application(std::string name, int width, int height);
        void destroy();
        void startFrame();
        void bindPipeline(bool textures);
        void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, uint32_t numOfIndices,std::vector<vk::WriteDescriptorSet> descriptorWrites, bool hasDescriptors);
        void endFrame();
        bool isWindowOpen(){return window.isWindowOpen();}
        void setClearValue(vk::ClearValue newClearValue){clearValue=newClearValue;};
        void waitIdle(){device.waitIdle();};
        Window window{};
        Logger logger{};
        PhysicalDevice physicalDevice{};
        Device device{};
        Queue queue{};
        Surface surface{};
        Swapchain swapchain{};
        Fence fence[NumFramesInFlight]{};
        CommandPool commandPool[NumFramesInFlight]{};
        CommandBuffer commandBuffer[NumFramesInFlight]{};
        GraphicsPipeline graphicsPipeline{};
        vk::DescriptorSetLayout pushDescriptorSetLayout{};
        BindlessTextures bindlessTextures{};
        vk::ShaderEXT vertexShader{};
        vk::ShaderEXT textureFragmentShader{};
        vk::ShaderEXT nonTextureFragmentShader{};
        vk::ShaderStageFlagBits shaderStages[2] = {vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment};
        vk::ClearValue clearValue;
        std::array<vk::ShaderEXT,2> textureShaders{};
        std::array<vk::ShaderEXT,2> nonTextureShaders{};
        vk::PipelineLayout pipelineLayout{};
        DepthBuffer depthBuffer{};
        int currentFrame = 0;
        uint32_t imageIndex = 0;
    };
}
