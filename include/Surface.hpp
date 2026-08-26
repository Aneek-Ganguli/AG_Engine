#pragma once
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"

namespace AG_EngineV2::Core {
    class Surface {
    public:
        Surface(){};
        Surface(vk::Instance instance, GLFWwindow* window);
        vk::SurfaceKHR surface{};

        void destroySurface(vk::Instance instance);
    };
}
