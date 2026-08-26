#include <Surface.hpp>
using namespace AG_EngineV2::Core;

Surface::Surface(vk::Instance instance, GLFWwindow* window) {
    if (!glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface)) == VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void Surface::destroySurface(vk::Instance instance) {
    instance.destroySurfaceKHR(surface);
}
