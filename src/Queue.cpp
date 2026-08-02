#include "Queue.hpp"
#include "PhysicalDevice.hpp"
#include <iostream>

#include "Window.hpp"
#include "Device.hpp"

using namespace AG_EngineV2::Core;

Queue::Queue(vk::PhysicalDevice* physicalDevice, Device* device) {
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice->getQueueFamilyProperties();

    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            // graphicsQueue = physicalDevice.;
            graphicsQueueFamilyIndex = i;
            if (debug){std::cout << "Selected Queue Family Index: " << i << std::endl;}
            break;
        }
    }

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.setQueueFamilyIndex(graphicsQueueFamilyIndex)
                   .setQueueCount(1)
                    .setPQueuePriorities(new float(1.0f));

    const char* deviceExtentions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.setQueueCreateInfoCount(1)
                    .setPQueueCreateInfos(&queueCreateInfo)
                    .setEnabledExtensionCount(1)
                    .setPEnabledExtensionNames( deviceExtentions  );


    device->device = physicalDevice->createDevice(deviceCreateInfo);

}

