#pragma once
// #define  VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"

namespace AG_EngineV2 {
    namespace Core {
        class Logger {
        public:
            Logger(vk::Instance instance);
            Logger(){};
            void cleanUp(vk::Instance instance);
            vk::detail::DispatchLoaderDynamic dldi{};


        private:
            vk::DebugUtilsMessengerEXT debugMessenger{nullptr};

            vk::DebugUtilsMessengerEXT createMessenger(vk::Instance instance);
        };
    }
}
