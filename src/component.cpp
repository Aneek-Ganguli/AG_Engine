#include <glm/glm.hpp>

#include "Component.hpp"

Transform3D initializeTransform3D(vec3 position,vec3 scale,vec3 rotate,float rotateAngle){
    Transform3D t3D;
    t3D.rotateAngle = rotateAngle;
    t3D.position = position;
    t3D.scale = scale;
    t3D.rotate = rotate;
    // glm_vec3_copy(position,t3D.position);
    // glm_vec3_copy(scale,t3D.scale);
    // glm_vec3_copy(rotate,t3D.rotate);
    return t3D;
}

void Texture::create(const char *p_fileName,Window* window) {
    surface = loadImage(p_fileName, 4);
    if (!surface) {
        printf("Failed to load texture image '%s'\n", p_fileName);
        return;
    }

    texture = createTexture(surface, window);
    if (!texture) {
        printf("Error creating GPU texture: %s\n", SDL_GetError());
        return;
    }

    // --- Texture staging + upload info ---
    const Uint32 texBytes = (surface->w * surface->h * 4);
    textureTransferBuffer = createTransferBuffer(texBytes, window);
    if (!textureTransferBuffer) {
        printf("Error creating texture staging buffer: %s\n", SDL_GetError());
        return;
    }

    void *texMem = SDL_MapGPUTransferBuffer(window->device, textureTransferBuffer, false);
    if (!texMem) {
        printf("Error mapping texture staging buffer: %s\n", SDL_GetError());
        return;
    }
    if (!surface->pixels) {
        printf("Loaded surface has NULL pixels\n");
        SDL_UnmapGPUTransferBuffer(window->device, textureTransferBuffer);
        return;
    }
    memcpy(texMem, surface->pixels, texBytes);
    SDL_UnmapGPUTransferBuffer(window->device, textureTransferBuffer);

    // textureRegion = (SDL_GPUTextureRegion){0};
    textureRegion.texture = texture;
    textureRegion.w = surface->w;
    textureRegion.h = surface->h;
    textureRegion.d = 1;

    // textureTransferInfo;
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    textureSamplerBinding.texture = texture;
    textureSamplerBinding.sampler = window->sampler;
}

void Texture::upload(Window* window) {
    if (textureTransferInfo.transfer_buffer == nullptr) {
        std::cerr << "ERROR textureTransferInfo is NULL" << std::endl;
    }

    uploadTexture(&textureTransferInfo, &textureRegion, window);
}

void Texture::bind(Window* window,int slotNum,int numBinding) {
    SDL_BindGPUFragmentSamplers(window->renderPass, 0, &textureSamplerBinding, 1);
}

void Texture::destroy(Window *window) {

    if (textureTransferBuffer) {
        SDL_ReleaseGPUTransferBuffer(window->device, textureTransferBuffer);
        textureTransferBuffer = NULL;
    }
    if (texture) {
        SDL_ReleaseGPUTexture(window->device, texture);
        texture = NULL;
    }
    if (surface) {
        SDL_DestroySurface(surface); // assuming SDL_Surface*
        surface = NULL;
    }
}
