#pragma once
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"


namespace AG_EngineV2::Core {
    static vk::SurfaceFormatKHR surfaceFormat;
    class Surface {
    public:
        Surface(){};
        Surface(vk::Instance instance,vk::PhysicalDevice physical_device, GLFWwindow* window);
        vk::SurfaceKHR surface{};

        vk::SurfaceCapabilitiesKHR surfaceCapabilities{};

        void destroySurface(vk::Instance instance);
    };
}
