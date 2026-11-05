#include "Application.hpp"

#include "shaders/fragmentShader.h"
#include "shaders/vertexShader.h"
#include "shaders/nonTextureFragmentShader.h"

using namespace AG_EngineV2;
using namespace AG_EngineV2::Core;

AG_EngineV2::Application::Application(std::string name, int width, int height)
    :window(width, height, name.c_str()),
     logger(window.getInstance()),
     physicalDevice(window.getInstance()),
     device(),
     queue(&physicalDevice.physicalDevice,&device),
     surface(window.getInstance(),physicalDevice.physicalDevice,window.window),
     swapchain(device.device,surface.surface,physicalDevice.physicalDevice,surface.surfaceCapabilities, NumFramesInFlight),
     graphicsPipeline()
{
    for (int i = 0; i < NumFramesInFlight; i++) {
        fence[i]  = Fence(device.device);
        commandPool[i] = CommandPool(device.device,queue.graphicsQueueFamilyIndex);
        commandBuffer[i] = CommandBuffer(device.device,commandPool[i].commandPool);
    }

    DescriptorSet uniformDescriptorSet(device.device, vk::DescriptorType::eUniformBuffer,0);
    DescriptorSet imageDescriptorSet(device.device, vk::DescriptorType::eCombinedImageSampler,1);

    std::array<vk::DescriptorSetLayoutBinding,2> descriptorSetLayoutBinding;
    descriptorSetLayoutBinding[0] = uniformDescriptorSet.layoutBinding;
    descriptorSetLayoutBinding[1] = imageDescriptorSet.layoutBinding;
    uniformDescriptorSet.destroy(device.device);
    imageDescriptorSet.destroy(device.device);

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR)
              .setBindingCount(static_cast<uint32_t>(descriptorSetLayoutBinding.size()))
              .setPBindings(descriptorSetLayoutBinding.data());
    pushDescriptorSetLayout = device.device.createDescriptorSetLayout(layoutInfo);



    vk::ShaderCreateInfoEXT shaderCreateInfo{};
    //vert
    shaderCreateInfo.setCodeType(vk::ShaderCodeTypeEXT::eSpirv)
                    .setCodeSize(shader_vert_spv_len)
                    .setPCode( reinterpret_cast<const uint32_t*>(shader_vert_spv))
                    .setPName("main")
                    .setStage(vk::ShaderStageFlagBits::eVertex)
                    .setSetLayouts(pushDescriptorSetLayout)
                    .setSetLayoutCount(1)
                    .setNextStage(vk::ShaderStageFlagBits::eFragment);

    vertexShader = device.device.createShaderEXT(shaderCreateInfo,nullptr,logger.dldi).value;

    //frag
    shaderCreateInfo.setNextStage({})
                    .setCodeSize(shader_frag_spv_len)
                    .setPCode(reinterpret_cast<const uint32_t*>(shader_frag_spv))
                    .setStage(vk::ShaderStageFlagBits::eFragment)
                    .setPName("main")
                    .setNextStage({});

    textureFragmentShader = device.device.createShaderEXT(shaderCreateInfo,nullptr,logger.dldi).value;

    shaderCreateInfo.setCodeSize(nonTexture_shader_frag_spv_len)
                    .setPCode(reinterpret_cast<const uint32_t*>(nonTexture_shader_frag_spv))
                    .setStage(vk::ShaderStageFlagBits::eFragment)
                    .setPName("main");
                    // .setNextStage({})
                    // .setSetLayouts(descriptorSetLayout)
                    // .setSetLayoutCount(1);
    nonTextureFragmentShader = device.device.createShaderEXT(shaderCreateInfo,nullptr,logger.dldi).value;

    textureShaders = { vertexShader, textureFragmentShader };
    nonTextureShaders = { vertexShader, nonTextureFragmentShader};

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(1)
                            .setPSetLayouts(&pushDescriptorSetLayout);


    pipelineLayout = device.device.createPipelineLayout(pipelineLayoutCreateInfo);
    if (pipelineLayout == nullptr) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    depthBuffer = DepthBuffer(device.device,&queue, physicalDevice.physicalDevice, window.width, window.height);

}

void Application::destroy() {
    depthBuffer.destroy(device.device);
    device.device.destroyDescriptorSetLayout(pushDescriptorSetLayout);
    device.device.destroyPipelineLayout(pipelineLayout);
    device.device.destroyShaderEXT(nonTextureFragmentShader, nullptr,logger.dldi);
    device.device.destroyShaderEXT(vertexShader,nullptr,logger.dldi);
    device.device.destroyShaderEXT(textureFragmentShader,nullptr,logger.dldi);
    for (int i = 0; i < NumFramesInFlight; i++) {
        fence[i].destroyFence(device.device);
        commandPool[i].destroy(device.device);
    }
    swapchain.destroy(device.device,NumFramesInFlight);
    surface.destroySurface(window.getInstance());
    device.cleanUp();
    logger.cleanUp(window.getInstance());
    window.cleanUp();
}

void Application::startFrame() {
    window.pollEvents();
    fence[currentFrame].waitAndResetFences(device.device);

    imageIndex = swapchain.acquireImages(device.device,currentFrame);

    commandPool[currentFrame].resetCommandPool(device.device);

    commandBuffer[currentFrame].begin();
    swapchain.transitionImageLayoutToColour(commandBuffer[currentFrame].commandBuffer[0], imageIndex);
    commandBuffer[currentFrame].pipelineBarrier(1, swapchain.transitionToColourImageBarrier);

    commandBuffer[currentFrame].beginRendering(swapchain.imageViews[imageIndex], window.width, window.height, clearValue);




    vk::DeviceSize offsets[] = { 0 };

}

void Application::bindPipeline(bool textures) {
    if (textures){
        graphicsPipeline.createDefaultPipeline(commandBuffer[currentFrame].commandBuffer[0],logger.dldi,
       shaderStages,textureShaders,window.width, window.height, depthBuffer.depthAttachmentInfo);
    }
    else {
        graphicsPipeline.createDefaultPipeline(commandBuffer[currentFrame].commandBuffer[0],logger.dldi,
      shaderStages,nonTextureShaders,window.width, window.height, depthBuffer.depthAttachmentInfo);
    }
}


void Application::draw(VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, uint32_t numOfIndices,std::vector<vk::WriteDescriptorSet> descriptorWrites, bool hasDescriptors) {
    if (hasDescriptors) {
        commandBuffer[currentFrame].commandBuffer[0].pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorWrites.size(), descriptorWrites.data(), logger.dldi);
    }
    commandBuffer[currentFrame].commandBuffer[0].setVertexInputEXT(1, &vertexBuffer->bindingDesc,
    vertexBuffer->attribDesc.size(), vertexBuffer->attribDesc.data(),logger.dldi);
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer[currentFrame].commandBuffer[0].bindVertexBuffers(0, 1, &vertexBuffer->vertexBuffer.buffer, offsets);
    commandBuffer[currentFrame].commandBuffer[0].bindIndexBuffer(indexBuffer->indexBuffer.buffer,0,vk::IndexType::eUint16);
    commandBuffer[currentFrame].commandBuffer[0].drawIndexed(static_cast<uint32_t>(numOfIndices), 1, 0, 0, 0);
}

void Application::endFrame() {
    commandBuffer[currentFrame].endRendering();

    swapchain.transitionImageLayoutToPresent(commandBuffer[currentFrame].commandBuffer[0], imageIndex);
    commandBuffer[currentFrame].pipelineBarrier(1, swapchain.transitionToPresentImageBarrier);

    commandBuffer[currentFrame].end();

    queue.submitQueue(&swapchain.acquireSemaphore[currentFrame], vk::PipelineStageFlagBits::eColorAttachmentOutput, &swapchain.releaseSemaphore[currentFrame],fence[currentFrame].fence,commandBuffer[currentFrame].commandBuffer[0]);
    queue.presentQueue(&swapchain.releaseSemaphore[currentFrame], &swapchain.swapchain, &imageIndex);

    currentFrame = (currentFrame + 1) % NumFramesInFlight;
}

