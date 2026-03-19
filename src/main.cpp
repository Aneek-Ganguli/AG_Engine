#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>


int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("Im an american",800,600);

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        window.endFrame();
    }

    window.cleanUp();
    return 0;
}