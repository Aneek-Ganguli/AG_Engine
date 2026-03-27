#pragma once
#include <functional>
#include <GLFW/glfw3.h>

#include  <vulkan/vulkan.hpp>

#include "Logger.hpp"
#include "PerFrameData.hpp"
#include "VertexInfo.hpp"

#ifdef DEBUG
    static bool debug = true;
#else
    static bool debug = false;
#endif

static constexpr uint32_t NUM_FRAMES_IN_FLIGHT = 2u,NUM_SWAPCHAIN_IMAGES = 3u;

class Window {
public:
    Window(const char *p_title, int width, int p_height);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isWindowOpen();
    void cleanUp();

    void startFrame();
    void endFrame();

    PerFrameData* getCurrentFrameData(){return currentFrameData;}

    vk::Device* getDevice(){return &device;}
    vk::PhysicalDevice* getPhysicalDevice(){return &physicalDevice;}

    vk::CommandPool createCommandPool();
    std::vector<vk::CommandBuffer> createCommandBuffer(vk::CommandPool commandPool);

    vk::Queue* getGraphicsQueue(){return &queue;}

private:
    const char* title;
    int width, height;
    GLFWwindow* window{};
    Logger logger;
    vk::Instance instance{};
    vk::SurfaceKHR surface{};
    uint32_t imageCount{};
    vk::SubmitInfo submitInfo{};
    vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput ;
    vk::PresentInfoKHR presentInfo{};


    void createGLFWwindow();
    void createDevice();
    void createSwapchain();
    vk::Fence createFence();
    uint32_t physicalDeviceCount{};
    vk::PhysicalDeviceProperties properties{};
    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};
    vk::Queue queue{};
    vk::Semaphore presentSemaphore{};

    vk::SurfaceFormatKHR actualSurfaceFormat{};
    //Swapchain
    vk::SwapchainKHR handle{};
    std::vector<vk::Image> images{};
    std::vector<vk::Semaphore> imagePresentSemaphore{};
    std::vector<vk::ImageView> imageViews{};

    uint32_t queueFamilyIndex;

    uint32_t imageIndex = 0;

    PerFrameData frameData[NUM_FRAMES_IN_FLIGHT]{};
    PerFrameData* currentFrameData = nullptr;
    uint8_t frameIndex = 0u;

    void createFrameData();
    void destroyFrameData();

    vk::Pipeline graphicsPipeline{};
    void createGraphicsPipeline();

    vk::ShaderModule vertexShader{};
    vk::ShaderModule fragmentShader{};
    void createShaderModules();

    vk::PipelineLayout pipelineLayout;

    VertexInfo vertexInfo{};
};

