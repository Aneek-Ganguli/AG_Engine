#include "Event.hpp"
#include <imgui/imgui.h>

#include <imgui/imgui_impl_sdl3.h>

using namespace AG_Engine;

bool Event::pollEvent() {
    return SDL_PollEvent(&event);
}

void Event::update() {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
        running = false;
    }
}
