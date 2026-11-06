#include "BindlessTextures.hpp"

using namespace AG_EngineV2;

BindlessTextures::BindlessTextures(vk::Device device) {
    layoutFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
    layoutFlagsInfo.pBindingFlags = bindingFlags.data();

    binding.binding = 1;
    binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    binding.descriptorCount = 1024; // Size of your large bindless array
    binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    layoutInfo2.pNext = &layoutFlagsInfo;
    layoutInfo2.bindingCount = 1;
    layoutInfo2.pBindings = &binding;
    layoutInfo2.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo2);

    poolSize.type = vk::DescriptorType::eCombinedImageSampler;
    poolSize.descriptorCount = 1024;

    // Mandatory flag for descriptor indexing pools
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    descriptorPool = device.createDescriptorPool(poolInfo);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    // 2. Allocate the descriptor set from the device
    // allocateDescriptorSets returns a std::vector<vk::DescriptorSet>
    allocatedSets = device.allocateDescriptorSets(allocInfo);
    descriptorSet = allocatedSets[0];
}

void BindlessTextures::destroy(vk::Device device) {
    device.destroyDescriptorSetLayout(descriptorSetLayout);
    device.destroyDescriptorPool(descriptorPool);
}

void BindlessTextures::bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout) {
    commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout,
            1,                  // firstSet
            1,                  // descriptorSetCount
            &descriptorSet,     // pDescriptorSets
            0, nullptr          // dynamic offsets
            );
}