#include <Fence.hpp>

using namespace AG_EngineV2::Core;
Fence::Fence(vk::Device device) {
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    fence = device.createFence(fenceCreateInfo);
}


void Fence::waitAndResetFences(vk::Device device) {
    if (device.waitForFences(1,&fence,vk::True,std::numeric_limits<uint64_t>::max())!=vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    if(device.resetFences(1,&fence) != vk::Result::eSuccess){
        throw std::runtime_error("Failed to reset fence!");
    }
}