#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Window.hpp"

namespace AG_Engine {
    struct Texture {
    public:
        Texture(const char* p_fileName,Window* window);
        Texture(SDL_FColor p_color):color(p_color),enable(false){}
        void upload(Window* window);
        void bind(Window* window,int slotNum,int numBinding);
        void destroy(Window* window);
        bool enable = false;
        SDL_FColor color{};

    private:
        SDL_GPUTexture *texture{};
        SDL_GPUTransferBuffer *textureTransferBuffer{};
        SDL_Surface* surface{};
        void* textureTransferMem{};
        SDL_GPUTextureTransferInfo textureTransferInfo{};
        SDL_GPUTextureRegion textureRegion{};
        SDL_GPUTextureSamplerBinding textureSamplerBinding{};
    };
}