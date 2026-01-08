#pragma once
#include <GLFW/glfw3.h>

#include  <vulkan/vulkan.hpp>

#include "Logger.hpp"

#ifdef DEBUG
    static bool debug = true;
#else
    static bool debug = false;
#endif

class Window {
public:
    Window(const char *p_title, int width, int p_height);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isWindowOpen();
    void cleanUp();

    void newFrame();

private:
    const char* title;
    int width, height;
    GLFWwindow* window{};
    Logger logger;
    vk::Instance instance{};

    vk::SurfaceKHR surface{};

    uint32_t imageCount{};

    vk::Fence fence{};

    vk::SubmitInfo submitInfo{};

    vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput ;

    vk::PresentInfoKHR presentInfo{};

    void createGLFWwindow();
    void createDevice();
    void createSwapchain();
    void createFence();

    uint32_t physicalDeviceCount{};
    vk::PhysicalDeviceProperties properties{};
    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};
    vk::Queue queue{};

    vk::Semaphore acquireSemaphore{};
    vk::Semaphore releaseSemaphore{};

    vk::SwapchainKHR handle{};
    std::vector<vk::Image> images{};
    std::vector<vk::Semaphore> imageReadySemaphore{};

    vk::CommandBuffer commandBuffer{};
    vk::CommandPool commandPool{};

    uint32_t queueFamilyIndex;
};
