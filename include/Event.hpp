#pragma once
#include <SDL3/SDL.h>

namespace AG_Engine{
    static struct ImGuiIO* io{};
    class Event {
    public:
        SDL_Event event;

        bool pollEvent();
        void update();
        bool running = true;

    };
}