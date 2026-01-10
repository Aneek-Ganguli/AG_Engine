#include <cassert>

#include <vulkan/vulkan.hpp>
#include <iostream>
#include  <Window.hpp>


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
    uint32_t imageCount = std::max(3u, surfaceCaps.minImageCount);
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

    // Framebuffer size
    int width = 0, height = 0;
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

    imageReadySemaphore.resize(imageCount);

    vk::SemaphoreCreateInfo semaphoreCI{};

    for (auto& semaphore : imageReadySemaphore) {
        semaphore = device.createSemaphore(semaphoreCI);
    }
    acquireSemaphore = device.createSemaphore(semaphoreCI);
}


void Window::createFence() {
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    if(device.createFence(&fenceCreateInfo,nullptr,&fence)!=vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fence!");
    }
}


Window::Window(const char* p_title,int p_width,int p_height):title(p_title),width(p_width),height(p_height),logger() {


    uint32_t version = VK_MAKE_API_VERSION(0,1,0,0);
    vkEnumerateInstanceVersion(&version);


    vk::ApplicationInfo appInfo = vk::ApplicationInfo(title,VK_MAKE_VERSION(1, 0, 0),"AG_Engine Vulkan",
        VK_MAKE_VERSION(1, 0, 0),VK_API_VERSION_1_0);
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

    createFence();

    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolCreateInfo.flags =  vk::CommandPoolCreateFlagBits::eTransient;

    if (device.createCommandPool(&commandPoolCreateInfo,nullptr,&commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create command pool!");
    }

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = 1;


    commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo);

    if (commandBuffer.empty()) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
    // if (!= vk::Result::eSuccess) {
        // throw std::runtime_error("Failed to allocate command buffers!");
    // }


}

bool Window::isWindowOpen() {
    return !glfwWindowShouldClose(window);
}

void Window::cleanUp() {

    // Stop GPU execution
    device.waitIdle();

    // Command buffers & sync
    device.destroyCommandPool(commandPool);

    for (auto semaphore : imageReadySemaphore) {
        device.destroySemaphore(semaphore);
    }

    device.destroySemaphore(acquireSemaphore);
    device.destroyFence(fence);

    // Swapchain
    device.destroySwapchainKHR(handle);

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
        vk::KHRSwapchainExtensionName
    };

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = std::size(extensions);
    deviceCreateInfo.ppEnabledExtensionNames = extensions;

    device = physicalDevice.createDevice(deviceCreateInfo);
    if (device == nullptr) {
        throw std::runtime_error("Failed to create device!");
    }

    queue = device.getQueue(queueFamilyIndex, 0);




}



void Window::startFrame() {

    if (device.waitForFences(1,&fence,vk::True,std::numeric_limits<uint64_t>::max())!=vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    if(device.resetFences(1,&fence) != vk::Result::eSuccess){
        throw std::runtime_error("Failed to reset fence!");
    }



    auto acquire = device.acquireNextImageKHR(
        handle,
        std::numeric_limits<uint64_t>::max(),
        acquireSemaphore,
        VK_NULL_HANDLE
    );


    if (acquire.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to new frame");
    }

    imageIndex = acquire.value;



    device.resetCommandPool(commandPool,{});

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    if (commandBuffer.data()->begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin command buffer!");
    }

    // commandBuffer.data()->beginRendering();

}

void Window::endFrame() {

    // commandBuffer.data()->endRendering();

    commandBuffer.data()->end();

    releaseSemaphore = imageReadySemaphore[imageIndex];

    submitInfo = NULL;
    submitInfo.sType = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &acquireSemaphore;
    submitInfo.pWaitDstStageMask = &pipelineStageFlags;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &releaseSemaphore;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer[0];

    if(queue.submit(1,&submitInfo,fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit queue");
    }

    presentInfo = vk::PresentInfoKHR(1,&releaseSemaphore,1,&handle,&imageIndex);


    if(queue.presentKHR(&presentInfo) != vk::Result::eSuccess) {
         throw std::runtime_error("Failed to present queue");
    }
}
