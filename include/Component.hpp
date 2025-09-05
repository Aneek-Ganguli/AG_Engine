#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Window.hpp"
#include "VertexData.hpp"

using namespace glm;

struct  Transform3D{
public:
    Transform3D(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle);
    Transform3D():position(NULL),M(NULL),scale(NULL),rotate(NULL),rotateAngle(NULL),transform(NULL){}
    UBO* getUBOData();
    Uint32 getUBOSize();
    void translate(glm::mat4 projection);
private:
    vec3 position{},scale{},rotate{};
    mat4 M{};
    float rotateAngle{};
    UBO transform{};
};

struct Texture {
public:
    void create(const char* p_fileName,Window* window);
    void upload(Window* window);
    void bind(Window* window,int slotNum,int numBinding);
    void destroy(Window* window);

private:
    SDL_GPUTexture *texture{};
    // SDL_GPUTexture* depthTexture{};
    SDL_GPUTransferBuffer *textureTransferBuffer{};
    SDL_Surface* surface{};
    void* textureTransferMem{};
    SDL_GPUTextureTransferInfo textureTransferInfo{};
    SDL_GPUTextureRegion textureRegion{};
    SDL_GPUTextureSamplerBinding textureSamplerBinding{};
};



// ok so i want to get the z and calculate thhe maximum x,y and clamp it
