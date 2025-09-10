#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Component.hpp"
#include "Component.hpp"
#include "Component.hpp"
#include "Component.hpp"
#include "Component.hpp"
#include "Window.hpp"
#include "glm/ext/matrix_clip_space.hpp"

Transform3D::Transform3D(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle)
:position(p_position),rotate(p_rotation),scale(p_scale),rotateAngle(glm::radians(p_rotateAngle)),M(NULL),transform(NULL){};

UBO* Transform3D::getUBOData() {
    return &transform;
}

Uint32 Transform3D::getUBOSize() {
    return sizeof(transform);
}

void Transform3D::translate(glm::mat4 view,glm::mat4 projection,float deltaTime) {

    // P = glm::perspective(fov,(float)windowWidth/windowHeight,0.1f,1000.0f);
    M = glm::mat4(1.0f);
    // translate (glm::translate returns a new matrix)
    // std::cout << position.x << position.y << position.z << std::endl
    M = glm::translate(M, position);
    // scale
    M = glm::scale(M, scale);
    glm::vec3 rot0 = {0,0,0};
    if (rotate != rot0 || rotateAngle != 0) {
        M = glm::rotate(M, rotateAngle * deltaTime, rotate);
    }

    // multiply P * M -> MVP
    transform.mvp = projection * view * M;
    // print_mat4(M);
}

void Texture::create(const char *p_fileName,Window* window) {
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
}

void Texture::upload(Window* window) {
    if (textureTransferInfo.transfer_buffer == nullptr) {
        std::cerr << "ERROR textureTransferInfo is NULL" << std::endl;
    }

    window->uploadTexture(&textureTransferInfo, &textureRegion);
}

void Texture::bind(Window* window,int slotNum,int numBinding) {
    SDL_BindGPUFragmentSamplers(window->getRenderPass(), slotNum, &textureSamplerBinding, numBinding);
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
