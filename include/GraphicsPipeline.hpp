#pragma once
#include <vulkan/vulkan.hpp>


namespace AG_EngineV2 {
    // A "Graphics pipeline", uses shader object extension"
    // This will be the default one
    // make new class for every new pipeline
    class GraphicsPipeline {
    public:
        void createDefaultPipeline(vk::CommandBuffer commandBuffer, vk::detail::DispatchLoaderDynamic dldi,vk::ArrayProxy<const vk::ShaderStageFlagBits> stages, vk::ArrayProxy<const vk::ShaderEXT> shaders, int windowWidth, int windowHeight);
    };
}
