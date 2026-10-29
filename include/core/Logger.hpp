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


        private:
            vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
            vk::detail::DispatchLoaderDynamic dldi{};

            vk::DebugUtilsMessengerEXT createMessenger(vk::Instance instance);
        };
    }
}
