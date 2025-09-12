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


struct Window{
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
	SDL_GPUTexture* createTexture(SDL_Surface *surface);

	void createDepthStencilTexture();

	SDL_GPUSampler* createGPUSampler();
	void uploadTexture(SDL_GPUTextureTransferInfo* textureTransferInfo,SDL_GPUTextureRegion* textureRegion);
	void cleanUp();
	SDL_GPUDevice* getGPUDevice(){return device;}
	SDL_GPUTransferBufferLocation createTransferBufferLocation(SDL_GPUTransferBuffer* transferBuffer,Uint32 offset);
	SDL_GPUBufferRegion createBufferRegion(Uint32 size,SDL_GPUBuffer* buffer);
	SDL_GPUBufferBinding createBufferBinding(SDL_GPUBuffer* buffer);
	glm::mat4 projection;
	glm::mat4 view = glm::mat4(1.0f);
	SDL_GPUSampler* getSampler(){return sampler;};
	SDL_GPURenderPass* getRenderPass(){return renderPass;};
	SDL_GPUCommandBuffer* getCommandBuffer(){return commandBuffer;};

	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

	void keyboadInput(SDL_Event &e, float deltaTime);

	float moveSpeed = 3.0f,lookSensitivity = 0.5f, pitch{},yaw{};

	vec2 mouseRel{};

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
	int oldWidth = 800,oldHeight = 600;
    float fov{};
	SDL_GPUTexture* depthTexture{};
} Window;


SDL_Surface* loadImage(const char* imageFilename, int desiredChannels);


void print_mat4(mat4 m);