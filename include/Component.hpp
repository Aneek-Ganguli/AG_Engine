#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Window.hpp"
#include "VertexData.hpp"

using namespace glm;

struct  Transform3D{
    vec3 position,scale,rotate;
    mat4 M;
    float rotateAngle;
    UBO transform;
};

struct Texture {
public:
    void create(const char* p_fileName,Window* window);
    void upload(Window* window);
    void bind(Window* window,int slotNum,int numBinding);
    void destroy(Window* window);

private:
    SDL_GPUTexture *texture{};
    SDL_GPUTransferBuffer *textureTransferBuffer{};
    SDL_Surface* surface{};
    void* textureTransferMem{};
    SDL_GPUTextureTransferInfo textureTransferInfo{};
    SDL_GPUTextureRegion textureRegion{};
    SDL_GPUTextureSamplerBinding textureSamplerBinding{};
};

enum SHAPE{
    SQUARE,
    CIRCLE,
    TRIANGLE,
    RECTANGLE,
    CUSTOM
};

struct ShapeData {
    SHAPE shape;
    Transform3D transform;
};


// ok so i want to get the z and calculate thhe maximum x,y and clamp it
