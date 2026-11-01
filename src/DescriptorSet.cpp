#include "DescriptorSet.hpp"

using namespace AG_EngineV2;
DescriptorSet::DescriptorSet(vk::Device device) {
    layoutBinding.setBinding(0)
                 .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                 .setDescriptorCount(1)
                 .setStageFlags(vk::ShaderStageFlagBits::eAll);


    layoutCreateInfo.setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR)
                    .setBindingCount(1)
                    .setPBindings(&layoutBinding);

    descriptorSetLayout = device.createDescriptorSetLayout(layoutCreateInfo);
    if (descriptorSetLayout == nullptr) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }


    pipelineLayoutCreateInfo.setSetLayoutCount(1)
                            .setPSetLayouts(&descriptorSetLayout);

    pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
    if (pipelineLayout == nullptr) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

}

void DescriptorSet::destroy(vk::Device device) {
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
}

