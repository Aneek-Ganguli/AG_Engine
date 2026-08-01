#include <Window.hpp>


int main() {
    AG_EngineV2::Core::Window window(800, 600, "VK Project V2");


    while (!window.isWindowOpen()) {
        window.pollEvents();
    }

    window.cleanUp();

    return 0;
}