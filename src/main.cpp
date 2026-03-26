#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"

Entity entity{};
void rendering(vk::CommandBuffer commandBuffer) {
    vk::Buffer vertexBuffers[] = {entity.vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0,1,vertexBuffers,offsets);
    commandBuffer.draw(3, 1, 0, 0);
}

int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    entity = Entity(&window,vertices);

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        window.doWhileRendering(&rendering);

        window.endFrame();
    }

    entity.cleanUp(&window);
    window.cleanUp();
    return 0;
}