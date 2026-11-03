#include "DescriptorSet.hpp"

using namespace AG_EngineV2;
DescriptorSet::DescriptorSet(vk::Device device,vk::DescriptorType type, int binding) {
    layoutBinding.setBinding(binding)
                 .setDescriptorType(type)
                 .setDescriptorCount(1)
                 .setStageFlags(vk::ShaderStageFlagBits::eAll);


    layoutCreateInfo.setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR)
                    .setBindingCount(1)
                    .setPBindings(&layoutBinding);

    descriptorSetLayout = device.createDescriptorSetLayout(layoutCreateInfo);
    if (descriptorSetLayout == nullptr) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }


}

void DescriptorSet::destroy(vk::Device device) {

    device.destroyDescriptorSetLayout(descriptorSetLayout);
}

