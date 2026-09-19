#include "GraphicsPipeline.hpp"
using namespace AG_EngineV2::Core;

GraphicsPipeline::GraphicsPipeline(vk::Device device) {
    vk::RenderingInfo renderingInfo;

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    graphicsPipelineCreateInfo.setPNext(&renderingInfo);

    graphicsPipeline = device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo).value;
    // vkCreateGraphicsPipelines();
}
