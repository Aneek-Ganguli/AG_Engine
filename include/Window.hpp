#pragma once
#include <functional>
#include <GLFW/glfw3.h>

#include  <vulkan/vulkan.hpp>

#include "DepthBuffer.hpp"
#include "Logger.hpp"
#include "PerFrameData.hpp"
#include "VertexInfo.hpp"

#ifdef DEBUG
    static bool debug = true;
#else
    static bool debug = false;
#endif

static constexpr uint32_t NUM_FRAMES_IN_FLIGHT = 2u,NUM_SWAPCHAIN_IMAGES = 3u;
uint8_t static frameIndex = 0u;
static int windowWidth, windowHeight;
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

    vk::DescriptorSetLayout getTextureDescriptorSetLayout(){return textureDescriptorSetLayout;}
    vk::DescriptorSetLayout getNoTextureDescriptorSetLayout(){return noTextureDescriptorSetLayout;}

    vk::PipelineLayout getTexturePipelineLayout(){return texturePipelineLayout;}
    vk::PipelineLayout getNoTexturePipelineLayout(){return noTexturePipelineLayout;}

    void bindTexturePipeline(){currentFrameData->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, textureGraphicsPipeline);}
    void bindNoTexturePipeline(){currentFrameData->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, noTextureGraphicsPipeline);}
    int width,height;
    void pollEvents(){glfwPollEvents();}

    std::array<float,4>clearColor{0.0f, 0.0f, 0.0f, 0.0f};
private:
    const char* title;
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
    uint32_t  imageIndex = 0;
    vk::SurfaceFormatKHR actualSurfaceFormat{};
    //Swapchain
    vk::SwapchainKHR handle{};
    std::vector<vk::Image> images{};
    std::vector<vk::Semaphore> imagePresentSemaphore{};
    std::vector<vk::ImageView> imageViews{};

    uint32_t queueFamilyIndex;


    PerFrameData frameData[NUM_FRAMES_IN_FLIGHT]{};
    PerFrameData* currentFrameData = nullptr;


    void createFrameData();
    void destroyFrameData();

    void createGraphicsPipeline();
    // Shape
    vk::Pipeline textureGraphicsPipeline{};
    vk::DescriptorSetLayout  textureDescriptorSetLayout{};
    vk::PipelineLayout  texturePipelineLayout;

    // No textures only shape/colour
    vk::Pipeline noTextureGraphicsPipeline{};
    vk::DescriptorSetLayout  noTextureDescriptorSetLayout{};
    vk::PipelineLayout  noTexturePipelineLayout;

    vk::ShaderModule vertexShader{};
    vk::ShaderModule textureFragmentShader{};
    vk::ShaderModule noTextureFragmentShader{};
    void createShaderModules();

    VertexInfo vertexInfo{};
    // VertexInfo2 vertexInfo2{};

    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    void createDescriptorSetLayout();

    std::vector<vk::Framebuffer> framebuffers;
    void createFramebuffers();

    vk::RenderPass renderPass;
    void createRenderPass();

    DepthBuffer depthBuffer;
};

