#include <Shader.hpp>

using namespace AG_EngineV2;

Shader::Shader(vk::Device device, void *code, uint32_t size, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, vk::ShaderStageFlagBits currentStage, vk::ShaderStageFlagBits nextStage, vk::detail::DispatchLoaderDynamic dldi)
    :dldi(dldi)
{
    vk::ShaderCreateInfoEXT shaderCreateInfo{};
    shaderCreateInfo.setCodeType(vk::ShaderCodeTypeEXT::eSpirv)
                .setCodeSize(size)
                .setPCode( reinterpret_cast<const uint32_t*>(code))
                .setPName("main")
                .setStage(currentStage)
                .setSetLayouts(descriptorSetLayouts)
                .setSetLayoutCount(descriptorSetLayouts.size())
                .setNextStage(nextStage);
    auto result = device.createShaderEXT(shaderCreateInfo,nullptr,dldi);
    shader = result.value;
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create VkShaderEXT object!");
    }
}

void Shader::destroy(vk::Device device) {
    device.destroyShaderEXT(shader,nullptr,dldi);
}