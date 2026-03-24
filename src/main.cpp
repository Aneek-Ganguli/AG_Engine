#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

void rendering(vk::CommandBuffer commandBuffer) {
    commandBuffer.draw(3, 1, 0, 0);
}

int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("Im an american",800,600);

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        window.doWhileRendering(&rendering);

        window.endFrame();
    }

    window.cleanUp();
    return 0;
}