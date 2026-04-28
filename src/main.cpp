#include "Texture.hpp"
#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"

int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);


    std::cout << "Window variables " << window.width << " " << window.height << std::endl;
    windowWidth = window.width;
    windowHeight = window.height;


    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };


    Texture text("res/resource.jpg",&window);
    std::cout << "Window Global variables " << windowWidth << " " << windowHeight << std::endl;
    Entity entity = Entity(&window,vertices,indices,&text);



    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        entity.updateUniformBuffer(frameIndex);
        entity.draw(&window);

        window.endFrame();
    }

    text.cleanUp(&window);
    entity.cleanUp(&window);
    window.cleanUp();
    return 0;
}
