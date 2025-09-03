#pragma once
#include <stdio.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
// #define CGLTF_IMPLEMENTATION
// #include <cgltf/cgltf.h>

#include "VertexData.hpp"

using namespace glm;

static const char* path = NULL;

static float windowHeight,windowWidth,fov;

// static glm::mat4 P;


typedef struct Window{
    SDL_Window *window;
    SDL_GPUDevice *device;
    SDL_GPUCommandBuffer *commandBuffer;
    SDL_GPUCommandBuffer *copyCommandBuffer;
    SDL_GPURenderPass *renderPass;
    SDL_GPUTexture *swapchainTexture;
    SDL_GPUGraphicsPipeline *pipeline;
    SDL_GPUShader *vertexShader;
    SDL_GPUShader *fragmentShader;
    SDL_GPUCopyPass *copyPass;
    SDL_GPUSampler* sampler;
    int width,height;
    float fov;
	glm::mat4 projection;
} Window;

struct Window createWindow(const char* title,int width,int height);
void createGraphicsPipeline(Window* window,float Fov);
void newFrame(struct Window *window);
void endFrame(struct Window *window);
SDL_GPUShader* loadShader(
	Window* window,
	const char* shaderFilename,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount
);
SDL_GPUBuffer *createBuffer(Uint32 size, SDL_GPUBufferUsageFlags usage, struct Window *window);
void startCopyPass(struct Window *window);
void endCopyPass(struct Window *window);
SDL_GPUTransferBuffer *createTransferBuffer(Uint32 size, struct Window *window);
void *createTransferMem(SDL_GPUTransferBuffer *transferBuffer, void *data, Uint32 size, struct Window *window);
SDL_GPUTransferBufferLocation createTransferBufferLocation(SDL_GPUTransferBuffer *transferBuffer, Uint32 offset);
SDL_GPUBufferRegion createBufferRegion(Uint32 size, SDL_GPUBuffer *buffer);
void uploadBuffer(SDL_GPUTransferBufferLocation *transferBufferLocation, SDL_GPUBufferRegion *bufferRegion, struct Window *window);
SDL_GPUBufferBinding createBufferBinding(SDL_GPUBuffer *buffer);
SDL_Surface* loadImage(const char* imageFilename, int desiredChannels);
SDL_GPUTexture* createTexture(SDL_Surface* surface,Window* window);
SDL_GPUSampler* createGPUSampler(Window* window);
void uploadTexture(SDL_GPUTextureTransferInfo* textureTransferInfo,SDL_GPUTextureRegion* textureRegion,
	Window* window);
void cleanUp(Window* window);


void print_mat4(mat4 m);