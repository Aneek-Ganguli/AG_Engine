#include "CommmandPool.hpp"

using namespace AG_EngineV2::Core;

CommandPool::CommandPool(vk::Device device, uint32_t queueFamilyIndex) {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
                         .setFlags(vk::CommandPoolCreateFlagBits::eTransient);
    if(device.createCommandPool(&commandPoolCreateInfo,nullptr,&commandPool) != vk::Result::eSuccess){
        throw std::runtime_error("Failed to create command pool");
    }
}

void CommandPool::resetCommandPool(vk::Device device) {
    device.resetCommandPool(commandPool, {});
}

void CommandPool::destroy(vk::Device device) {
    device.destroyCommandPool(commandPool);
}