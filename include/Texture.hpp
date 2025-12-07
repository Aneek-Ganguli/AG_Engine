#pragma once
#include <SDL3/SDL.h>

#include "Window.hpp"

namespace AG_Engine {
    struct Texture {
    public:
        Texture(const char* p_fileName,Window* window);
        Texture(const glm::vec4 p_color):color({p_color}){}
        void upload(Window* window);
        void bind(Window* window,int slotNum,int numBinding);
        void destroy(Window* window);
        bool enable = false;
        ColorUniform color{};


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