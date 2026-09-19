#pragma once
#include "vulkan/vulkan.hpp"

namespace  AG_EngineV2::Core {
    class Fence {
    public:
        Fence(){};
        Fence(vk::Device device);
        vk::Fence fence;
        void waitAndResetFences(vk::Device device);
    };

}
