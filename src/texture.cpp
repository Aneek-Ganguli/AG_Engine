#include <SDL3/SDL.h>
#include  <SDL3_image/SDL_image.h>

#include "Texture.hpp"
using namespace AG_Engine;

Texture::Texture(const char *p_fileName, Window *window) {
    surface = loadImage(p_fileName, 4);
    if (!surface) {
        printf("Failed to load texture image '%s'\n", p_fileName);
        return;
    }

    texture = window->createTexture(surface);
    if (!texture) {
        printf("Error creating GPU texture: %s\n", SDL_GetError());
        return;
    }


    const Uint32 texBytes = (surface->w * surface->h * 4);
    textureTransferBuffer = window->createTransferBuffer(texBytes);
    if (!textureTransferBuffer) {
        printf("Error creating texture staging buffer: %s\n", SDL_GetError());
        return;
    }

    void *texMem = SDL_MapGPUTransferBuffer(window->getGPUDevice(), textureTransferBuffer, false);
    if (!texMem) {
        printf("Error mapping texture staging buffer: %s\n", SDL_GetError());
        return;
    }
    if (!surface->pixels) {
        printf("Loaded surface has NULL pixels\n");
        SDL_UnmapGPUTransferBuffer(window->getGPUDevice(), textureTransferBuffer);
        return;
    }
    memcpy(texMem, surface->pixels, texBytes);
    SDL_UnmapGPUTransferBuffer(window->getGPUDevice(), textureTransferBuffer);

    // textureRegion = (SDL_GPUTextureRegion){0};
    textureRegion.texture = texture;
    textureRegion.w = surface->w;
    textureRegion.h = surface->h;
    textureRegion.d = 1;

    // textureTransferInfo;
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    textureSamplerBinding.texture = texture;
    textureSamplerBinding.sampler = window->getSampler();
    enable = true;

    // glm::vec4 color = glm::vec4(1,1,1,1);
    color.color = {1,1,1,1};
}

void Texture::upload(Window* window) {
    if (enable){
        if (textureTransferInfo.transfer_buffer == nullptr) {
            std::cerr << "ERROR textureTransferInfo is NULL" << std::endl;
        }

        window->uploadTexture(&textureTransferInfo, &textureRegion);
    }
}

void Texture::bind(Window* window,int slotNum,int numBinding) {
    if (enable) {
        SDL_BindGPUFragmentSamplers(window->getRenderPass(), slotNum, &textureSamplerBinding, numBinding);
    }

    // SDL_PushGPUFragmentUniformData(window->getCommandBuffer(),3,&color,sizeof(color));
}

void Texture::destroy(Window *window) {

    if (textureTransferBuffer) {
        SDL_ReleaseGPUTransferBuffer(window->getGPUDevice(), textureTransferBuffer);
        textureTransferBuffer = nullptr;
    }
    if (texture) {
        SDL_ReleaseGPUTexture(window->getGPUDevice(), texture);
        texture = nullptr;
    }
    if (surface) {
        SDL_DestroySurface(surface); // assuming SDL_Surface*
        surface = nullptr;
    }
}
