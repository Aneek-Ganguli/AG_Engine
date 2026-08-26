#include <Surface.hpp>
using namespace AG_EngineV2::Core;

Surface::Surface(vk::Instance instance,vk::PhysicalDevice physicalDevice, GLFWwindow* window) {
    if (!glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface)) == VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    if(physicalDevice.getSurfaceCapabilitiesKHR(surface, &surfaceCapabilities) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to get surface capabilities!");
    }
}

void Surface::destroySurface(vk::Instance instance) {
    instance.destroySurfaceKHR(surface);
}
