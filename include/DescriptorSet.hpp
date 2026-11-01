#pragma once
#include <vulkan/vulkan.hpp>

#include "core/Device.hpp"

namespace AG_EngineV2 {
    struct DescriptorSet {
        DescriptorSet(){};
        DescriptorSet(vk::Device device);
        vk::DescriptorSetLayoutBinding layoutBinding{};
        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};

        vk::DescriptorSetLayout descriptorSetLayout;


        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};

        vk::PipelineLayout pipelineLayout;

        void destroy(vk::Device device);

    };
}
