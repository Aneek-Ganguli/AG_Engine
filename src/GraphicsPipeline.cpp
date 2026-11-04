#include "GraphicsPipeline.hpp"

using namespace AG_EngineV2;

void GraphicsPipeline::createDefaultPipeline(vk::CommandBuffer commandBuffer, vk::DispatchLoaderDynamic dldi,vk::ArrayProxy<const vk::ShaderStageFlagBits> stages, vk::ArrayProxy<const vk::ShaderEXT> shaders, int windowWidth, int windowHeight,vk::RenderingAttachmentInfo &depth) {
    commandBuffer.bindShadersEXT(stages,shaders,dldi);
    commandBuffer.setViewportWithCount(vk::Viewport(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 0.0f, 1.0f));
    commandBuffer.setScissorWithCount(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(windowWidth, windowHeight)));
    commandBuffer.setRasterizerDiscardEnable(false);
    // commandBuffer.setVertexInputEXT( nullptr, nullptr,dldi);
    commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    commandBuffer.setPrimitiveRestartEnable(false);
    commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1,dldi);
    vk::SampleMask sampleMask = 0xFFFFFFFF;
    commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1,sampleMask,dldi);
    commandBuffer.setAlphaToCoverageEnableEXT(false,dldi);
    commandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill,dldi);
    commandBuffer.setCullModeEXT(vk::CullModeFlagBits::eNone,dldi);
    commandBuffer.setFrontFaceEXT(vk::FrontFace::eCounterClockwise,dldi);
    commandBuffer.setDepthTestEnable(true);
    commandBuffer.setDepthWriteEnable(true);
    commandBuffer.setDepthBiasEnable(true);
    commandBuffer.setDepthBias( 0.0f, 0.0f, 0.0f);
    commandBuffer.setDepthCompareOp(vk::CompareOp::eLess);
    commandBuffer.setStencilTestEnable(true);
    commandBuffer.setStencilCompareMask( vk::StencilFaceFlagBits::eFrontAndBack, 0xFF);
    commandBuffer.setStencilWriteMask( vk::StencilFaceFlagBits::eFrontAndBack, 0xFF);
    commandBuffer.setStencilReference( vk::StencilFaceFlagBits::eFrontAndBack, 0);
    commandBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eFrontAndBack,
        vk::StencilOp::eKeep, // failOp
        vk::StencilOp::eKeep, // passOp
        vk::StencilOp::eKeep, // depthFailOp
        vk::CompareOp::eAlways,
        dldi
    );
    commandBuffer.setColorBlendEnableEXT(0,vk::False,dldi);
    commandBuffer.setColorWriteMaskEXT(0,vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,dldi);
}
