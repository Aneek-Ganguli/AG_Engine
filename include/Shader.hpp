#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2 {
    class Shader {
        vk::detail::DispatchLoaderDynamic dldi{};
    public:
        Shader() = default;
        Shader(vk::Device device, void *code, uint32_t size, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, vk::ShaderStageFlagBits currentStage, vk::ShaderStageFlagBits nextStage, vk::detail::DispatchLoaderDynamic dldi);
        void destroy(vk::Device device);
        vk::ShaderEXT shader{};
    };
}