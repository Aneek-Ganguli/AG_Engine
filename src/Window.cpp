#include "Window.hpp"

#include <iostream>
#include <ostream>

using namespace AG_EngineV2::Core;



std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}
bool checkExtention(std::vector<const char*> requestedExtensions) {
    std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
    if (debug){
        std::cout << "Supported extensions: " << std::endl;
        for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
            std::cout << supportedExtension.extensionName << std::endl;
        }
    }

    bool found;
    for (const char* extension : requestedExtensions) {
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

    return true;
}
void Window::extensionInitialization() {
    extensions = getRequiredExtensions();
    if (debug){
        extensions.push_back(   "VK_EXT_debug_utils");
        std::cout << checkExtention(extensions) << std::endl;
    }
}

bool checkLayers(std::vector<const char*> requestedLayers) {
    bool found;
    std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

    if (debug){
        std::cout << "Device can support the following layers:\n";
        for (vk::LayerProperties supportedLayer : supportedLayers) {
            std::cout << '\t' << supportedLayer.layerName << '\n';
        }
    }


    for (const char* layer : requestedLayers) {
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
void Window::layerInitialization() {
    if (debug) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        std::cout << checkLayers(layers) << std::endl;
    }
}

void Window::createInstance() {
    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("Vulkan Application")
           .setApplicationVersion(VK_MAKE_VERSION(0, 0, 1))
           .setPEngineName("AG_EngineV2")
           .setEngineVersion(VK_MAKE_VERSION(0, 0, 1))
           .setApiVersion(VK_API_VERSION_1_3);

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo)
              .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
              .setPpEnabledLayerNames(layers.data())
              .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
              .setPpEnabledExtensionNames(extensions.data());

    instance = vk::createInstance(createInfo);
    if (instance == nullptr) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
    }
}

void Window::createGLFWwindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
    }
}


Window::Window(int width, int height, const char* title)  {

    createGLFWwindow(width,height,title);

    extensionInitialization();
    layerInitialization();

    createInstance();

    // logger = Logger(instance);

}

void Window::cleanUp() {
    // logger.cleanUp(instance);
    instance.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
}
