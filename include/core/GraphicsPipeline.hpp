#pragma once
#include <vulkan/vulkan.hpp>


namespace AG_EngineV2::Core {
    class GraphicsPipeline {
    public:
        GraphicsPipeline(vk::Device device);
        vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::Pipeline graphicsPipeline;

    };
}
