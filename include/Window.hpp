#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "Logger.hpp"
#include "PhysicalDevice.hpp"

#ifndef NO_DEBUG
#define debug true
#else
#define debug false
#endif

namespace AG_EngineV2 {
    namespace Core{
        class Window {
        public:
            Window(int width, int height, const char* title);
            void cleanUp();
            bool isWindowOpen(){return glfwWindowShouldClose(window);};
            void pollEvents(){glfwPollEvents();};
        private:
            GLFWwindow* window;
            vk::Instance instance;
            Logger logger;

            std::vector<const char*> extensions;
            void extensionInitialization();

            std::vector<const char*> layers;
            void layerInitialization();

            void createInstance();

            void createGLFWwindow(int width, int height, const char* title);

            PhysicalDevice physicalDevice;
        };
    }
}