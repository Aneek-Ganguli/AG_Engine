#include <iostream>
#include <vulkan/vulkan.hpp>
#include "Logger.hpp"
using namespace AG_EngineV2::Core;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {

    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

Logger::Logger(vk::Instance& instance) {
    dldi = vk::detail::DispatchLoaderDynamic(instance,vkGetInstanceProcAddr);
    debugMessenger = createMessenger(instance);
}

vk::DebugUtilsMessengerEXT Logger::createMessenger(vk::Instance& instance) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            //severity
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,


            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugCallback,
            nullptr
        );

    return instance.createDebugUtilsMessengerEXT(createInfo,nullptr,dldi);
}

void Logger::cleanUp(vk::Instance& instance) {
    // vkDestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
    instance.destroyDebugUtilsMessengerEXT(debugMessenger,nullptr,dldi);
}


