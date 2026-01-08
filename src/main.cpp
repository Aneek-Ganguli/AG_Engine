#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>


int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("Hello In a British Accent",800,600);

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.newFrame();
    }

    window.cleanUp();
    return 0;
}