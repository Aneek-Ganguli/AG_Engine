#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"


int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    Entity entity = Entity(&window,vertices);

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        entity.draw(&window);

        window.endFrame();
    }

    entity.cleanUp(&window);
    window.cleanUp();
    return 0;
}