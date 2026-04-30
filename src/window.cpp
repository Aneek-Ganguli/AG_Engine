#include <cassert>

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <Window.hpp>

#include "DepthBuffer.hpp"
#include "vertexShader.h"
#include "fragmentShader.h"

void glfwErrorCallback(int error, const char* description){
    std::cerr << "[GLFW ERROR] (" << error << "): " << description << std::endl;
}

bool supported(std::vector<const char*>& p_extensions, std::vector<const char*>& p_layers) {

    std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

    if (debug){
        std::cout << "Supported extensions: " << std::endl;
        for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
            std::cout << "\t" << supportedExtension.extensionName << std::endl;
        }
    }

    bool found;
    for (const char* extension : p_extensions) {
        found = false;
        for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
            if (strcmp(extension, supportedExtension.extensionName) == 0) {
                found = true;
                if (debug) std::cout << "Extension \"" << extension << "\" is supported!\n";
            }
        }
        if (!found) {
            if (debug){ throw std::runtime_error("Extension is not supported!\n"); }
            return false;
        }
    }

    std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

    if (debug){
        std::cout << "Device can support the following layers:\n";
        for (vk::LayerProperties supportedLayer : supportedLayers) {
            std::cout << '\t' << supportedLayer.layerName << '\n';
        }
    }

    for (const char* layer : p_layers) {
        found = false;
        for (vk::LayerProperties supportedLayer : supportedLayers) {
            if (strcmp(layer, supportedLayer.layerName) == 0) {
                found = true;
                if (debug) std::cout << "Layer \"" << layer << "\" is supported!\n";
            }
        }
        if (!found) {
            if (debug) std::cout << "Layer \"" << layer << "\" is not supported!\n";
            return false;
        }
    }

    return true;
}

// ── NEW: render pass creation ─────────────────────────────────────────────────
void Window::createRenderPass() {
    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format         = findDepthFormat(&physicalDevice);
    depthAttachment.samples        = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;


    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format         = actualSurfaceFormat.format;
    colorAttachment.samples        = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dep{};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass    = 0;
    dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                              vk::PipelineStageFlagBits::eLateFragmentTests;

    dep.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                              vk::PipelineStageFlagBits::eEarlyFragmentTests;

    dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                               vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo rpInfo{};
    rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    rpInfo.pAttachments    = attachments.data();
    rpInfo.subpassCount    = 1;
    rpInfo.pSubpasses      = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies   = &dep;



    if (device.createRenderPass(&rpInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

// ── NEW: framebuffer creation (one per swapchain image) ───────────────────────
void Window::createFramebuffers() {
    framebuffers.resize(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
        std::array<vk::ImageView, 2> attachments = {
            imageViews[i],
            depthBuffer.depthImageView // You need to have created this earlier!
        };

        vk::FramebufferCreateInfo fbInfo{};
        fbInfo.renderPass      = renderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments    = attachments.data();
        fbInfo.width           = static_cast<uint32_t>(windowWidth);
        fbInfo.height          = static_cast<uint32_t>(windowHeight);
        fbInfo.layers          = 1;

        if (device.createFramebuffer(&fbInfo, nullptr, &framebuffers[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void Window::createSwapchain() {
    vk::SurfaceCapabilitiesKHR surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    uint32_t imageCount = std::max(NUM_SWAPCHAIN_IMAGES, surfaceCaps.minImageCount);
    if (surfaceCaps.maxImageCount > 0) {
        imageCount = std::min(imageCount, surfaceCaps.maxImageCount);
    }

    std::vector<vk::SurfaceFormatKHR> surfaceFormats =
        physicalDevice.getSurfaceFormatsKHR(surface);

    vk::SurfaceFormatKHR surfaceFormat = surfaceFormats.front();
    for (const auto& candidate : surfaceFormats) {
        if (candidate.format == vk::Format::eB8G8R8A8Srgb &&
            candidate.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = candidate;
            break;
        }
    }

    actualSurfaceFormat = surfaceFormat;

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    // std::cout << width << " " << height << "\n";

    auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    if (presentModes.empty()) {
        throw std::runtime_error("No present mode found!");
    }

    vk::PresentModeKHR actualPresentMode = vk::PresentModeKHR::eFifo;
    for (auto e : presentModes) {
        if (e == vk::PresentModeKHR::eMailbox) {
            actualPresentMode = e;
            break;
        }
    }

    vk::SwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.surface          = surface;
    swapchainCI.minImageCount    = imageCount;
    swapchainCI.imageFormat      = surfaceFormat.format;
    swapchainCI.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainCI.imageExtent      = vk::Extent2D{
        static_cast<uint32_t>(windowWidth),
        static_cast<uint32_t>(windowHeight)
    };
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapchainCI.preTransform     = surfaceCaps.currentTransform;
    swapchainCI.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCI.presentMode      = actualPresentMode;
    swapchainCI.clipped          = VK_TRUE;
    swapchainCI.oldSwapchain     = VK_NULL_HANDLE;

    handle = device.createSwapchainKHR(swapchainCI);
    if (handle == nullptr) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    images = device.getSwapchainImagesKHR(handle);

    imagePresentSemaphore.resize(imageCount);
    vk::SemaphoreCreateInfo semaphoreCI{};
    for (auto& semaphore : imagePresentSemaphore) {
        semaphore = device.createSemaphore(semaphoreCI);
    }

    for (auto& image : images) {
        vk::ImageViewCreateInfo imageViewCI{};
        imageViewCI.sType            = vk::StructureType::eImageViewCreateInfo;
        imageViewCI.viewType         = vk::ImageViewType::e2D;
        imageViewCI.image            = image;
        imageViewCI.format           = surfaceFormat.format;
        imageViewCI.subresourceRange = vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        );
        imageViews.push_back(device.createImageView(imageViewCI));
    }
}

vk::Fence Window::createFence() {
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    vk::Fence fence{};
    if (device.createFence(&fenceCreateInfo, nullptr, &fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fence!");
    }
    return fence;
}

vk::CommandPool Window::createCommandPool() {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolCreateInfo.flags            = vk::CommandPoolCreateFlagBits::eTransient;

    vk::CommandPool commandPool{};
    if (device.createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create command pool!");
    }
    return commandPool;
}

std::vector<vk::CommandBuffer> Window::createCommandBuffer(vk::CommandPool commandPool) {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandPool        = commandPool;
    commandBufferAllocateInfo.level              = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = 1;

    std::vector<vk::CommandBuffer> commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo);
    if (commandBuffer.empty()) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
    return commandBuffer;
}

void Window::createFrameData() {
    for (auto& e : frameData) {
        e.commandPool   = createCommandPool();
        e.commandBuffer = *createCommandBuffer(e.commandPool).data();
        e.fence         = createFence();
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        e.acquireSemaphore = device.createSemaphore(semaphoreCreateInfo);
    }
}

void Window::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding            = 1;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.descriptorType     = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags         = vk::ShaderStageFlagBits::eFragment;

    uboLayoutBinding.binding         = 0;
    uboLayoutBinding.descriptorType  = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags      = vk::ShaderStageFlagBits::eVertex;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    if (device.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Window::destroyFrameData() {
    for (auto& e : frameData) {
        device.destroyCommandPool(e.commandPool);
        device.destroySemaphore(e.acquireSemaphore);
        device.destroyFence(e.fence);
    }
}

Window::Window(const char* p_title, int p_width, int p_height) : title(p_title), logger() {
    width  = p_width;
    height = p_height;
    windowWidth  = p_width;
    windowHeight = p_height;



    uint32_t version = VK_MAKE_API_VERSION(0, 1, 0, 0);
    vkEnumerateInstanceVersion(&version);

    // ── dropped to 1.2 — no dynamic rendering needed ──────────────────────────
    vk::ApplicationInfo appInfo = vk::ApplicationInfo(
        title, VK_MAKE_VERSION(1, 0, 0),
        "AG_Engine Vulkan", VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_2
    );

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    for (const char* e : extensions) std::cout << e << "\n";

    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

    extensions.push_back("VK_EXT_debug_utils");
    extensions.push_back("VK_KHR_surface");
    extensions.push_back("VK_KHR_win32_surface");

    if (!supported(extensions, layers)) {
        throw std::runtime_error("Either Validation Layers or GLFW Extensions aren't supported");
    }

    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(), &appInfo,
        layers.size(), layers.data(),
        extensions.size(), extensions.data()
    );

    if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create instance!");
    }

    createGLFWwindow();

    VkSurfaceKHR cSurface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, window, nullptr, &cSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    surface = cSurface;

    logger = Logger(instance);

    createDevice();
    createSwapchain();
    depthBuffer = DepthBuffer(&device,&physicalDevice, windowWidth, windowHeight);
    // ── render pass + framebuffers before pipeline ─────────────────────────────
    createRenderPass();
    createFramebuffers();

    createFrameData();
    createShaderModules();
    createDescriptorSetLayout();
    createGraphicsPipeline();

    vertexInfo = VertexInfo();
}

bool Window::isWindowOpen() {
    return !glfwWindowShouldClose(window);
}

void Window::cleanUp() {
    device.waitIdle();

    destroyFrameData();

    for (auto semaphore : imagePresentSemaphore) {
        device.destroySemaphore(semaphore);
    }

    depthBuffer.cleanUp(&device);

    // ── destroy framebuffers + render pass ────────────────────────────────────
    for (auto fb : framebuffers) {
        device.destroyFramebuffer(fb);
    }
    device.destroyRenderPass(renderPass);

    for (auto i : imageViews) {
        device.destroyImageView(i);
    }

    device.destroyShaderModule(vertexShader, nullptr);
    device.destroyShaderModule(fragmentShader, nullptr);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
    device.destroySwapchainKHR(handle);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyPipeline(graphicsPipeline);
    device.destroy();

    logger.cleanUp(instance);
    instance.destroySurfaceKHR(surface);
    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::createGLFWwindow() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "glfwInit failed. Life is pain." << std::endl;
        glfwTerminate();
        throw std::runtime_error("GLFW failed to initialize");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
    assert(window && "GLFW window creation failed!");
}

void Window::createDevice() {
    auto physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("No GPUs found!");
    }

    bool found = false;
    for (const auto& candidate : physicalDevices) {
        auto queueFamilies = candidate.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            bool supportsGraphics = static_cast<bool>(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);
            bool supportsPresent  = candidate.getSurfaceSupportKHR(i, surface);
            if (supportsGraphics && supportsPresent) {
                physicalDevice   = candidate;
                queueFamilyIndex = i;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!found) throw std::runtime_error("No suitable GPU found!");

    float queuePriority = 1.0f;

    const char* extensions[] = { vk::KHRSwapchainExtensionName };

    // ── removed VkPhysicalDeviceVulkan13Features entirely ─────────────────────

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount   = std::size(extensions);
    deviceCreateInfo.ppEnabledExtensionNames = extensions;
    // ── pNext is now nullptr, no 1.3 feature chain ────────────────────────────
    deviceCreateInfo.pNext = nullptr;

    device = physicalDevice.createDevice(deviceCreateInfo);
    if (device == nullptr) throw std::runtime_error("Failed to create device!");

    queue = device.getQueue(queueFamilyIndex, 0);
}

void Window::createShaderModules() {
    vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
    vertexShaderModuleCreateInfo.codeSize = shader_vert_spv_len;
    vertexShaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shader_vert_spv);

    if (device.createShaderModule(&vertexShaderModuleCreateInfo, nullptr, &vertexShader) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create vertex shader module!");
    }

    vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
    fragmentShaderModuleCreateInfo.codeSize = shader_frag_spv_len;
    fragmentShaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shader_frag_spv);

    if (device.createShaderModule(&fragmentShaderModuleCreateInfo, nullptr, &fragmentShader) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fragment shader module!");
    }
}

void Window::createGraphicsPipeline() {
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates    = dynamicStates.data();

    vk::PipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.vertexBindingDescriptionCount   = 1;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInfo.attributeDescriptions.size());
    vertexInput.pVertexBindingDescriptions      = &vertexInfo.bindingDescription;
    vertexInput.pVertexAttributeDescriptions    = vertexInfo.attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology               = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = vk::False;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable        = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode             = vk::PolygonMode::eFill;
    rasterizer.cullMode                = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace               = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable         = vk::False;
    rasterizer.lineWidth               = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable  = vk::False;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = vk::False;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable   = vk::False;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &colorBlendAttachment;

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts    = &descriptorSetLayout;
    if (device.createPipelineLayout(&layoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::PipelineShaderStageCreateInfo vertStage{};
    vertStage.stage  = vk::ShaderStageFlagBits::eVertex;
    vertStage.module = vertexShader;
    vertStage.pName  = "main";

    vk::PipelineShaderStageCreateInfo fragStage{};
    fragStage.stage  = vk::ShaderStageFlagBits::eFragment;
    fragStage.module = fragmentShader;
    fragStage.pName  = "main";

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;


    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    // ── removed pNext renderingInfo, added renderPass instead ─────────────────
    pipelineInfo.pNext               = nullptr;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = pipelineLayout;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.renderPass          = renderPass;  // ← the swap
    pipelineInfo.subpass             = 0;           // ← new

    auto result = device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void Window::startFrame() {
    currentFrameData = &frameData[frameIndex];

    if (device.waitForFences(1, &currentFrameData->fence, vk::True, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }
    if (device.resetFences(1, &currentFrameData->fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to reset fence!");
    }

    auto acquire = device.acquireNextImageKHR(
        handle,
        std::numeric_limits<uint64_t>::max(),
        currentFrameData->acquireSemaphore,
        VK_NULL_HANDLE
    );
    if (acquire.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire new frame");
    }
    imageIndex = acquire.value;

    device.resetCommandPool(currentFrameData->commandPool, {});

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    if (currentFrameData->commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin command buffer!");
    }

    // ── render pass begin replaces beginRendering + barriers ──────────────────
    vk::ClearValue clearValue{};
    clearValue.color = vk::ClearColorValue{std::array<float,4>{0.0f, 1.0f, 1.0f, 1.0f}};
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = vk::ClearColorValue{std::array<float,4>{0.0f, 1.0f, 1.0f, 1.0f}};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderPassBeginInfo rpBegin{};
    rpBegin.renderPass        = renderPass;
    rpBegin.framebuffer       = framebuffers[imageIndex];
    rpBegin.renderArea        = vk::Rect2D({0, 0}, {static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)});
    rpBegin.clearValueCount   = static_cast<uint32_t>(clearValues.size());
    rpBegin.pClearValues      = clearValues.data();

    currentFrameData->commandBuffer.beginRenderPass(&rpBegin, vk::SubpassContents::eInline);

    assert(graphicsPipeline != VK_NULL_HANDLE && "Pipeline is null!");
    assert(windowWidth > 0 && windowHeight > 0 && "Invalid dimensions!");

    vk::Viewport vp{};
    vp.x        = 0.0f;
    vp.y        = 0.0f;
    vp.width    = (float)windowWidth;
    vp.height   = (float)windowHeight;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;

    vk::Rect2D sc = vk::Rect2D(
        {0,0},
        {static_cast<uint32_t>(windowWidth),static_cast<uint32_t>(windowHeight)}
    );

    currentFrameData->commandBuffer.setViewport(0, 1, &vp);
    currentFrameData->commandBuffer.setScissor(0, 1, &sc);
    currentFrameData->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
}

void Window::endFrame() {
    // ── endRenderPass replaces endRendering + present barrier ─────────────────
    currentFrameData->commandBuffer.endRenderPass();
    currentFrameData->commandBuffer.end();

    presentSemaphore = imagePresentSemaphore[imageIndex];

    submitInfo        = NULL;
    submitInfo.sType  = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &currentFrameData->acquireSemaphore;
    submitInfo.pWaitDstStageMask    = &pipelineStageFlags;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &presentSemaphore;
    submitInfo.pNext                = nullptr;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &currentFrameData->commandBuffer;

    if (queue.submit(1, &submitInfo, currentFrameData->fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit queue");
    }

    presentInfo = vk::PresentInfoKHR(1, &presentSemaphore, 1, &handle, &imageIndex);
    if (queue.presentKHR(&presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present queue");
    }

    frameIndex = (frameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
}