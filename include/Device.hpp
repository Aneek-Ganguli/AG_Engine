#pragma once
#include <vulkan/vulkan.hpp>

namespace AG_EngineV2 {
    namespace Core{
        class Device {
        public:
            Device(){};
            // vk::Device* getDevicePtr(){return &device;};
            void cleanUp();
        // private:
            vk::Device device{};
        };
    }
}