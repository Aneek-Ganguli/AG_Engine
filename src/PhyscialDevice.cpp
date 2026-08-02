#include "PhysicalDevice.hpp"
#include <iostream>

#include "Window.hpp"

using namespace AG_EngineV2::Core;

PhysicalDevice::PhysicalDevice(vk::Instance instance) {
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    vk::PhysicalDevice selectedDevice;
    for (auto& device : devices) {
        vk::PhysicalDeviceProperties2 deviceProperties;
        device.getProperties2(&deviceProperties);
        if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu || deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            selectedDevice = device;
            if (debug){std::cout << "Selected Physical Device: " << deviceProperties.properties.deviceName << std::endl;}
            break;
        }
        // vkGetPhysicalDeviceProperties2(device, &deviceProperties);
    }
    physicalDevice = selectedDevice;
}

