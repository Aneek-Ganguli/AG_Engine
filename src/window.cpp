#include <cassert>

#include <vulkan/vulkan.hpp>
#include <iostream>
#include  <Window.hpp>
#include "vertexShader.h"
#include "fragmentShader.h"

void glfwErrorCallback(int error, const char* description){
    std::cerr << "[GLFW ERROR] (" << error << "): " << description << std::endl;
}

bool supported(std::vector<const char*>& p_extensions,std::vector<const char *>& p_layers) {

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
                if (debug)std::cout << "Extension \"" << extension << "\" is supported!\n";
            }
        }
        if (!found) {
            if (debug){throw std::runtime_error("Extension  is not supported!\n");}
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
                if (debug)std::cout << "Layer \"" << layer << "\" is supported!\n";
            }
        }
        if (!found) {
                if (debug)std::cout << "Layer \"" << layer << "\" is not supported!\n";
            return false;
        }
    }

    return true;
}



void Window::createSwapchain() {
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    vk::SurfaceCapabilitiesKHR surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    // Image count
    uint32_t imageCount = std::max(NUM_SWAPCHAIN_IMAGES, surfaceCaps.minImageCount);
    if (surfaceCaps.maxImageCount > 0) {
        imageCount = std::min(imageCount, surfaceCaps.maxImageCount);
    }

    // Surface formats
    std::vector<vk::SurfaceFormatKHR> surfaceFormats =
        physicalDevice.getSurfaceFormatsKHR(surface);



    // Pick preferred format
    vk::SurfaceFormatKHR surfaceFormat = surfaceFormats.front();
    for (const auto& candidate : surfaceFormats) {
        if (candidate.format == vk::Format::eB8G8R8A8Srgb &&
            candidate.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = candidate;
            break;
            }
    }

    actualSurfaceFormat = surfaceFormat;

    // Framebuffer size
    // int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    auto presentMode = physicalDevice.getSurfacePresentModesKHR(surface);
    if (presentMode.empty()) {
        throw std::runtime_error("No present mode found!");
    }

    vk::PresentModeKHR actualPresentMode = vk::PresentModeKHR::eFifo;
    for (auto e : presentMode) {
        if (e == vk::PresentModeKHR::eMailbox) {
            actualPresentMode = e;
            break;
        }
    }


    // Swapchain create info
    vk::SwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.surface = surface;
    swapchainCI.minImageCount = imageCount;
    swapchainCI.imageFormat = surfaceFormat.format;
    swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCI.imageExtent = vk::Extent2D{
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapchainCI.preTransform = surfaceCaps.currentTransform;
    swapchainCI.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCI.presentMode = actualPresentMode;
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.oldSwapchain = VK_NULL_HANDLE;



    // Create swapchain
    handle = device.createSwapchainKHR(swapchainCI);
    if (handle == nullptr) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    // device.getSwapchainImagesKHR(&handle,);

    images = device.getSwapchainImagesKHR(handle);

    semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
    semaphoreCreateInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    imagePresentSemaphore.resize(imageCount);

    vk::SemaphoreCreateInfo semaphoreCI{};

    for (auto& semaphore : imagePresentSemaphore) {
        semaphore = device.createSemaphore(semaphoreCI);//no info
    }



    for (auto& image : images) {
        vk::ImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = vk::StructureType::eImageViewCreateInfo;
        imageViewCI.viewType = vk::ImageViewType::e2D;
        imageViewCI.image = image;
        imageViewCI.format = surfaceFormat.format;
        imageViewCI.subresourceRange = vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
        );

        imageViews.push_back(device.createImageView(imageViewCI));
    }
}



vk::Fence Window::createFence() {
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    vk::Fence fence{};
    if(device.createFence(&fenceCreateInfo,nullptr,&fence)!=vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fence!");
    }

    return fence;
}

vk::CommandPool Window::createCommandPool() {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolCreateInfo.flags =  vk::CommandPoolCreateFlagBits::eTransient;

    vk::CommandPool commandPool{};
    if (device.createCommandPool(&commandPoolCreateInfo,nullptr,&commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create command pool!");
    }

    return commandPool;
}
std::vector<vk::CommandBuffer> Window::createCommandBuffer(vk::CommandPool commandPool) {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = 1;

    std::vector<vk::CommandBuffer> commandBuffer{};
    commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo);

    if (commandBuffer.empty()) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
    return commandBuffer;
}

void Window::createFrameData() {
    for (auto& e : frameData) {
        e.commandPool = createCommandPool();
        e.commandBuffer = *createCommandBuffer(e.commandPool).data();
        e.fence = createFence();
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        e.acquireSemaphore = device.createSemaphore(semaphoreCreateInfo);//no info can be {}
    }
}



void Window::destroyFrameData() {
    for (auto& e : frameData) {
        device.destroyCommandPool(e.commandPool);
        device.destroySemaphore(e.acquireSemaphore);
        device.destroyFence(e.fence);
    }
}


Window::Window(const char* p_title,int p_width,int p_height):title(p_title),width(p_width),height(p_height),logger() {


    uint32_t version = VK_MAKE_API_VERSION(0,1,0,0);
    vkEnumerateInstanceVersion(&version);


    vk::ApplicationInfo appInfo = vk::ApplicationInfo(title,VK_MAKE_VERSION(1, 0, 0),"AG_Engine Vulkan",
        VK_MAKE_VERSION(1, 0, 0),VK_API_VERSION_1_3);
    // appInfo.apiVersion = VK_API_VERSION_1_0;

    //create glfw extensions
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,glfwExtensions+glfwExtensionCount);

    for (const char* e : extensions) {
        std::cout << e << "\n" ;
    }
    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

    {
        extensions.push_back("VK_EXT_debug_utils");
        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_win32_surface");
    }

    // debug = true;
    if (!supported(extensions,layers)) {
        throw std::runtime_error("Either Validation Layers or GLFW Extensions arent supported");
    }
    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(),&appInfo,layers.size(),layers.data(),
        extensions.size(),extensions.data());

    if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create instance!");
    }

    createGLFWwindow();

    VkSurfaceKHR cSurface = VK_NULL_HANDLE;

    if (glfwCreateWindowSurface(instance, window, nullptr, &cSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    surface = cSurface;

    // vkDestroySurfaceKHR(instance, cSurface, nullptr);

    logger = Logger(instance);



    createDevice();

    createSwapchain();

    createFrameData();

    createShaderModules();
    createGraphicsPipeline();

    vertexInfo = VertexInfo();


}

bool Window::isWindowOpen() {
    return !glfwWindowShouldClose(window);
}


void Window::cleanUp() {

    // Stop GPU execution
    device.waitIdle();

    // Command buffers & sync

    destroyFrameData();

    for (auto semaphore : imagePresentSemaphore) {
        device.destroySemaphore(semaphore);
    }

    for (auto i : imageViews) {
        device.destroyImageView(i);
    }

    device.destroyShaderModule(vertexShader,nullptr);
    device.destroyShaderModule(fragmentShader,nullptr);


    // Swapchain
    device.destroySwapchainKHR(handle);

    device.destroyPipelineLayout(pipelineLayout);
    device.destroyPipeline(graphicsPipeline);


    // Device
    device.destroy();

    // Debug messenger
    logger.cleanUp(instance);

    // Surface & instance
    instance.destroySurfaceKHR(surface);
    instance.destroy();

    // Windowing
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

    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE , GLFW_FALSE);

    window = glfwCreateWindow(width,height,title,nullptr,nullptr);

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
            bool supportsGraphics =
                static_cast<bool>(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);

            bool supportsPresent =
                candidate.getSurfaceSupportKHR(i, surface);

            if (supportsGraphics && supportsPresent) {
                physicalDevice = candidate;
                queueFamilyIndex = i;
                found = true;
                break;
            }
        }

        if (found) break;
    }

    if (!found) {
        throw std::runtime_error("No suitable GPU found!");
    }


    float queuePriority = 1.0f;

    const char* extensions[] = {
        vk::KHRSwapchainExtensionName,
    };

    vk::PhysicalDeviceVulkan13Features enabled13{};
    enabled13.dynamicRendering = VK_TRUE;
    enabled13.synchronization2 = VK_TRUE;



    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = std::size(extensions);
    deviceCreateInfo.ppEnabledExtensionNames = extensions;
    deviceCreateInfo.pNext = &enabled13;

    device = physicalDevice.createDevice(deviceCreateInfo);
    if (device == nullptr) {
        throw std::runtime_error("Failed to create device!");
    }

    queue = device.getQueue(queueFamilyIndex, 0);

}

void Window::createShaderModules() {
    vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
    vertexShaderModuleCreateInfo.codeSize = shader_vert_spv_len;
    vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shader_vert_spv);

    if (device.createShaderModule(&vertexShaderModuleCreateInfo,nullptr,&vertexShader) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create vertex shader module!");
    }

    vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
    fragmentShaderModuleCreateInfo.codeSize = shader_frag_spv_len;
    fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shader_frag_spv);

    if (device.createShaderModule(&fragmentShaderModuleCreateInfo,nullptr,&fragmentShader) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fragment shader module!");
    }
}

void Window::createGraphicsPipeline() {
    // ALL local, no member variables
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates    = dynamicStates.data();

    vk::PipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInfo.attributeDescriptions.size());
    vertexInput.pVertexBindingDescriptions = &vertexInfo.bindingDescription;
    vertexInput.pVertexAttributeDescriptions = vertexInfo.attributeDescriptions.data();

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
    colorBlending.pAttachments    = &colorBlendAttachment; // safe, both local

    vk::PipelineLayoutCreateInfo layoutInfo{};
    if (device.createPipelineLayout(&layoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::Format colorFormat = actualSurfaceFormat.format;
    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.colorAttachmentCount    = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    vk::PipelineShaderStageCreateInfo vertStage{};
    vertStage.stage  = vk::ShaderStageFlagBits::eVertex;
    vertStage.module = vertexShader;
    vertStage.pName  = "main";

    vk::PipelineShaderStageCreateInfo fragStage{};
    fragStage.stage  = vk::ShaderStageFlagBits::eFragment;
    fragStage.module = fragmentShader;
    fragStage.pName  = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pNext               = &renderingInfo;
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
    pipelineInfo.renderPass          = nullptr;

    auto result = device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}


void Window::startFrame() {
    currentFrameData = &frameData[frameIndex];


    if (device.waitForFences(1,&currentFrameData->fence,vk::True,std::numeric_limits<uint64_t>::max())!=vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    if(device.resetFences(1,&currentFrameData->fence) != vk::Result::eSuccess){
        throw std::runtime_error("Failed to reset fence!");
    }



    auto acquire = device.acquireNextImageKHR(
        handle,
        std::numeric_limits<uint64_t>::max(),
        currentFrameData->acquireSemaphore,
        VK_NULL_HANDLE
    );


    if (acquire.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to new frame");
    }

    imageIndex = acquire.value;



    device.resetCommandPool(currentFrameData->commandPool,{});

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;


    if (currentFrameData->commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin command buffer!");
    }

    vk::ImageMemoryBarrier2 transitionToColorAttachmentBarrier{};
    transitionToColorAttachmentBarrier.image = images[imageIndex];
    transitionToColorAttachmentBarrier.subresourceRange = vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor,
        0,
        1,
        0,
        1
    );
    transitionToColorAttachmentBarrier.oldLayout = vk::ImageLayout::eUndefined;
    transitionToColorAttachmentBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    transitionToColorAttachmentBarrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
    transitionToColorAttachmentBarrier.srcAccessMask = vk::AccessFlagBits2::eMemoryRead;
    transitionToColorAttachmentBarrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    transitionToColorAttachmentBarrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;


    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &transitionToColorAttachmentBarrier;
    currentFrameData->commandBuffer.pipelineBarrier2(dependencyInfo);

    vk::ClearValue clearValue{};
    clearValue.color.float32[0] = 0.0f;
    clearValue.color.float32[1] = 1.0f;
    clearValue.color.float32[2] = 1.0f;
    clearValue.color.float32[3] = 1.0f;


    vk::RenderingAttachmentInfo  colorAttachmentInfo{};
    colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentInfo.imageView = imageViews[imageIndex];
    colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentInfo.clearValue =clearValue;

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea = vk::Rect2D(
        {0,0},
        {static_cast<uint32_t>(width),static_cast<uint32_t>(height)}
    );
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount =1;
    renderingInfo.pColorAttachments = &colorAttachmentInfo;



    currentFrameData->commandBuffer.beginRendering(&renderingInfo);

    // currentFrameData->commandBuffer.draw(3,1,0,0);

    assert(graphicsPipeline != VK_NULL_HANDLE && "Pipeline is null!");
    assert(width > 0 && height > 0 && "Invalid dimensions!");

    vk::Viewport vp{};
    vp.x        = 0.0f;
    vp.y        = 0.0f;
    vp.width    = (float)width;
    vp.height   = (float)height;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;

    vk::Rect2D sc{};
    sc.offset.x = 0.0f;
    sc.offset.y = 0.0f;

    sc.extent.width = (uint32_t)width;
    sc.extent.height = (uint32_t)height;

    currentFrameData->commandBuffer.setViewport(0, 1, &vp);
    currentFrameData->commandBuffer.setScissor(0, 1, &sc);

    currentFrameData->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

}

void Window::endFrame() {

    currentFrameData->commandBuffer.endRendering();

    vk::ImageMemoryBarrier2 transitionToPresent_src_Barrier{};
    transitionToPresent_src_Barrier.image = images[imageIndex];
    transitionToPresent_src_Barrier.subresourceRange = vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor,
        0,
        1,
        0,
        1
    );
    transitionToPresent_src_Barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    transitionToPresent_src_Barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    transitionToPresent_src_Barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    transitionToPresent_src_Barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    transitionToPresent_src_Barrier.dstAccessMask = {};
    transitionToPresent_src_Barrier.dstStageMask = {};

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &transitionToPresent_src_Barrier;
    currentFrameData->commandBuffer.pipelineBarrier2(dependencyInfo);

    currentFrameData->commandBuffer.end();

    presentSemaphore = imagePresentSemaphore[imageIndex];

    submitInfo = NULL;
    submitInfo.sType = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &currentFrameData->acquireSemaphore;
    submitInfo.pWaitDstStageMask = &pipelineStageFlags;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &presentSemaphore;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentFrameData->commandBuffer;

    if(queue.submit(1,&submitInfo,currentFrameData->fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit queue");
    }

    // queue.submit(submitInfo,fence);

    presentInfo = vk::PresentInfoKHR(1,&presentSemaphore,1,&handle,&imageIndex);


    if(queue.presentKHR(&presentInfo) != vk::Result::eSuccess) {
         throw std::runtime_error("Failed to present queue");
    }

    frameIndex = (frameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
}
