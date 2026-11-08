#pragma once
#include <vulkan/vulkan.hpp>


namespace AG_EngineV2 {
    class BindlessTextures {
    public:
        BindlessTextures() = default;
        BindlessTextures(vk::Device device);
        void destroy(vk::Device device);
        void bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
        std::array<vk::DescriptorBindingFlags, 1> bindingFlags = {
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind
        };

        vk::DescriptorSetLayoutBindingFlagsCreateInfo layoutFlagsInfo{};
        vk::DescriptorSetLayoutBinding binding{};
        vk::DescriptorSetLayoutCreateInfo layoutInfo2{};

        vk::DescriptorSetLayout descriptorSetLayout;

        vk::DescriptorPoolSize poolSize{};

        vk::DescriptorPoolCreateInfo poolInfo{};

        vk::DescriptorPool descriptorPool{};
        std::vector<vk::DescriptorSet> allocatedSets{};
        vk::DescriptorSet descriptorSet{};
    };
}