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
public:
	Window(const char* title,int width,int height);
	SDL_GPUShader* loadShader(const char* shaderFilename,Uint32 samplerCount,Uint32 uniformBufferCount,
	Uint32 storageBufferCount,Uint32 storageTextureCount);
	void startFrame();
	void endFrame();
	SDL_GPUBuffer* createBuffer(Uint32 size,SDL_GPUBufferUsageFlags usage );
	void startCopyPass();
	void endCopyPass();
	SDL_GPUTransferBuffer* createTransferBuffer(Uint32 size);
	void uploadBuffer(SDL_GPUTransferBufferLocation* transferBufferLocation, SDL_GPUBufferRegion* bufferRegion);
	void createGraphicsPipeline();
	void createPerspective(float p_fov);
	SDL_GPUTexture* createTexture(SDL_Surface* surface);
	SDL_GPUSampler* createGPUSampler();
	void uploadTexture(SDL_GPUTextureTransferInfo* textureTransferInfo,SDL_GPUTextureRegion* textureRegion);
	void cleanUp();
	SDL_GPUDevice* getGPUDevice(){return device;}
	SDL_GPUTransferBufferLocation createTransferBufferLocation(SDL_GPUTransferBuffer* transferBuffer,Uint32 offset);
	SDL_GPUBufferRegion createBufferRegion(Uint32 size,SDL_GPUBuffer* buffer);
	SDL_GPUBufferBinding createBufferBinding(SDL_GPUBuffer* buffer);
	glm::mat4 projection;
	SDL_GPUSampler* getSampler(){return sampler;};
	SDL_GPURenderPass* getRenderPass(){return renderPass;};
	SDL_GPUCommandBuffer* getCommandBuffer(){return commandBuffer;};

private:
    SDL_Window *window{};
    SDL_GPUDevice *device{};
    SDL_GPUCommandBuffer *commandBuffer{};
    SDL_GPUCommandBuffer *copyCommandBuffer{};
    SDL_GPURenderPass *renderPass{};
    SDL_GPUTexture *swapchainTexture{};
    SDL_GPUGraphicsPipeline *pipeline{};
    SDL_GPUShader *vertexShader{};
    SDL_GPUShader *fragmentShader{};
    SDL_GPUCopyPass *copyPass{};
    SDL_GPUSampler* sampler{};
    int width,height{};
    float fov{};
} Window;

// struct Window createWindow(const char* title,int width,int height);
// void createGraphicsPipeline(Window* window,float Fov);
// void newFrame(struct Window *window);
// void endFrame(struct Window *window);
// SDL_GPUShader* loadShader(
// 	Window* window,
// 	const char* shaderFilename,
// 	Uint32 samplerCount,
// 	Uint32 uniformBufferCount,
// 	Uint32 storageBufferCount,
// 	Uint32 storageTextureCount
// );
// SDL_GPUBuffer *createBuffer(Uint32 size, SDL_GPUBufferUsageFlags usage, struct Window *window);
// void startCopyPass(struct Window *window);
// void endCopyPass(struct Window *window);
// SDL_GPUTransferBuffer *createTransferBuffer(Uint32 size, struct Window *window);
// void *createTransferMem(SDL_GPUTransferBuffer *transferBuffer, void *data, Uint32 size, struct Window *window);
// SDL_GPUTransferBufferLocation createTransferBufferLocation(SDL_GPUTransferBuffer *transferBuffer, Uint32 offset);
// SDL_GPUBufferRegion createBufferRegion(Uint32 size, SDL_GPUBuffer *buffer);
// void uploadBuffer(SDL_GPUTransferBufferLocation *transferBufferLocation, SDL_GPUBufferRegion *bufferRegion, struct Window *window);
// SDL_GPUBufferBinding createBufferBinding(SDL_GPUBuffer *buffer);
SDL_Surface* loadImage(const char* imageFilename, int desiredChannels);
// SDL_GPUTexture* createTexture(SDL_Surface* surface,Window* window);
// SDL_GPUSampler* createGPUSampler(Window* window);
// void uploadTexture(SDL_GPUTextureTransferInfo* textureTransferInfo,SDL_GPUTextureRegion* textureRegion,
// 	Window* window);
// void cleanUp(Window* window);
//

void print_mat4(mat4 m);