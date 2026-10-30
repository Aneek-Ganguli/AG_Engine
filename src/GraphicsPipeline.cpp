#include "GraphicsPipeline.hpp"

using namespace AG_EngineV2;

void GraphicsPipeline::createDefaultPipeline(vk::CommandBuffer commandBuffer, vk::detail::DispatchLoaderDynamic dldi,vk::ArrayProxy<const vk::ShaderStageFlagBits> stages, vk::ArrayProxy<const vk::ShaderEXT> shaders, int windowWidth, int windowHeight) {
    commandBuffer.bindShadersEXT(stages,shaders,dldi);
    commandBuffer.setViewportWithCount(vk::Viewport(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 0.0f, 1.0f));
    commandBuffer.setScissorWithCount(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(windowWidth, windowHeight)));
    commandBuffer.setRasterizerDiscardEnable(false);
    commandBuffer.setVertexInputEXT( nullptr, nullptr,dldi);
    commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    commandBuffer.setPrimitiveRestartEnable(false);
    commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1,dldi);
    vk::SampleMask sampleMask = 0xFFFFFFFF;
    commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1,sampleMask,dldi);
    commandBuffer.setAlphaToCoverageEnableEXT(false,dldi);
    commandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill,dldi);
    commandBuffer.setCullModeEXT(vk::CullModeFlagBits::eNone,dldi);
    commandBuffer.setFrontFaceEXT(vk::FrontFace::eCounterClockwise,dldi);
    commandBuffer.setDepthTestEnable(false);
    commandBuffer.setDepthWriteEnable(false);
    commandBuffer.setDepthBiasEnable(false);
    commandBuffer.setStencilTestEnable(false);
    commandBuffer.setColorBlendEnableEXT(0,true,dldi);
    commandBuffer.setColorWriteMaskEXT(0,vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,dldi);
}
