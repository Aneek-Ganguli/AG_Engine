#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlgpu3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Event.hpp"
#include "shaderShape.h"
#include "shaderTexture.h"
#include "shaderVert.h"


#include "Window.hpp"
#include "VertexData.hpp"

using namespace AG_Engine;


Window::Window(const char* title,int width,int height,float p_fov):width(width),height(height){
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
	}

	float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	window = SDL_CreateWindow(title,width * main_scale,height * main_scale,window_flags);
	if(window == NULL){
		printf("Error creating window: %s\n",SDL_GetError());
	}
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,true,NULL);
	if(device == nullptr){
		printf("Error creating gpu device: %s: \n",SDL_GetError());
	}

	SDL_ClaimWindowForGPUDevice(device,window);

	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo init_info = {};
	init_info.Device = device;
	init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
	init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.
	init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;  // Only used in multi-viewports mode.
	init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
	ImGui_ImplSDLGPU3_Init(&init_info);

	path = SDL_GetBasePath();

	createDepthStencilTexture();

	// view = glm::lookAt(cameraPosition,cameraTarget,{0,1,0});

	view = glm::lookAt(
		cameraPos,   // eye (camera position)
		cameraTarget,// center (what you’re looking at)
		cameraUp     // up vector
	);

	vertexShader = loadShader(shader_vert_spv, shader_vert_spv_len,0, 2,
	                          0, 0, SDL_GPU_SHADERSTAGE_VERTEX);

	fragmentTextureShader = loadShader(texture_frag_spv, texture_frag_spv_len, 1, 0,
	                            0, 0, SDL_GPU_SHADERSTAGE_FRAGMENT);

	fragmentShapeTexture = loadShader(shape_frag_spv, shape_frag_spv_len, 0, 0,
								0, 0, SDL_GPU_SHADERSTAGE_FRAGMENT);


	SDL_GPUVertexBufferDescription vertexBufferDescription{};
	vertexBufferDescription.slot = 0;
	vertexBufferDescription.pitch = sizeof(VertexData);
	vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute vertexAttributes[2]{};
	vertexAttributes[0] = {
		.location = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = offsetof(struct VertexData,position),
	};
	vertexAttributes[1] = {
		.location = 1,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(struct VertexData,texCoord),
	};

	SDL_GPUVertexInputState vertexInput{};
	vertexInput.num_vertex_buffers = 1;
	vertexInput.num_vertex_attributes = 2;
	vertexInput.vertex_buffer_descriptions = &vertexBufferDescription;
    vertexInput.vertex_attributes =  vertexAttributes;

	SDL_GPUColorTargetBlendState colorTargetBlend{};
	colorTargetBlend.enable_blend = false;

	if (device == NULL) {
		std::cout << "Failed to create graphics pipeline." << SDL_GetError() << std::endl;
	}

	if (window == NULL) {
		std::cout << "Failed to create graphics pipeline." << SDL_GetError() << std::endl;
	}
	SDL_GPUColorTargetDescription colorTargetDescription{};
	colorTargetDescription.blend_state = colorTargetBlend;
	colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device, window);

	SDL_GPUGraphicsPipelineTargetInfo graphicsPipelineTargetInfo{};
	graphicsPipelineTargetInfo.num_color_targets = 1;
	graphicsPipelineTargetInfo.color_target_descriptions = &colorTargetDescription;
	graphicsPipelineTargetInfo.has_depth_stencil_target = true;
	graphicsPipelineTargetInfo.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;


	SDL_GPUDepthStencilState depthStencilState{};
	depthStencilState.enable_depth_test = true;
	depthStencilState.enable_depth_write = true;
	depthStencilState.compare_op = SDL_GPU_COMPAREOP_LESS;

	SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
	graphics_pipeline_create_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	graphics_pipeline_create_info.target_info = graphicsPipelineTargetInfo;
	graphics_pipeline_create_info.vertex_input_state = vertexInput;
	graphics_pipeline_create_info.vertex_shader = vertexShader;
	graphics_pipeline_create_info.fragment_shader = fragmentTextureShader;
	graphics_pipeline_create_info.depth_stencil_state = depthStencilState;

    texturePipeline = SDL_CreateGPUGraphicsPipeline(device,&graphics_pipeline_create_info);
    if(texturePipeline == NULL){
        printf("Erro graphics pipeline :%s\n", SDL_GetError());
	}

	graphics_pipeline_create_info.fragment_shader = fragmentShapeTexture;
	shapePipeline = SDL_CreateGPUGraphicsPipeline(device,&graphics_pipeline_create_info);
	if(shapePipeline == NULL){
		printf("Erro graphics pipeline :%s\n", SDL_GetError());
	}


	sampler = createSampler();
	// fov = Fov;
	windowWidth = width;
    windowHeight = height;
	fov = p_fov;
	projection = perspective(glm::radians(fov),(float)width/height,0.0001f,1000.0f);

	// startCopyPass();

}

SDL_GPUShader* Window::loadShader(
	void* code,
	size_t codeSize,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount,
	SDL_GPUShaderStage shaderStage) {


	SDL_GPUShaderCreateInfo shaderInfo{};
	shaderInfo.code = static_cast<const Uint8*>(code);
	shaderInfo.code_size = codeSize;
	shaderInfo.entrypoint = "main";
	shaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	shaderInfo.stage = shaderStage;
	shaderInfo.num_samplers = samplerCount;
	shaderInfo.num_uniform_buffers = uniformBufferCount;
	shaderInfo.num_storage_buffers = storageBufferCount;
	shaderInfo.num_storage_textures = storageTextureCount;
	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL)
	{
		SDL_Log("Failed to create shader!");
		SDL_free(code);
		return NULL;
	}

	// SDL_free(code);
	return shader;
}



void Window::startFrame() {
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Render();
	drawData = ImGui::GetDrawData();

	// Get current window size
	int newWidth, newHeight;
	SDL_GetWindowSize(window, &newWidth, &newHeight);

	// Only recreate depth texture if the window size changed
	if (newWidth != width || newHeight != height) {
		width = newWidth;
		height = newHeight;
		createDepthStencilTexture();
	}

	view = glm::lookAt(
		cameraPos,   // eye (camera position)
		cameraTarget,// center (what you’re looking at)
		cameraUp     // up vector
	);


	Uint32 flags = SDL_GetWindowFlags(window);

	if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)) {
		commandBuffer = SDL_AcquireGPUCommandBuffer(device);
		if (!commandBuffer) {
			printf("Error acquiring command buffer: %s\n", SDL_GetError());
		}

		if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, NULL, NULL)) {
			printf("Error acquiring swapchain texture: %s\n", SDL_GetError());
		}

		SDL_GPUColorTargetInfo colorTargetInfo{};
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		colorTargetInfo.clear_color = {1.0f, 1.0f, 1.0f, 1.0f};

		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
		depthStencilTargetInfo.texture = depthTexture;
		depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
		depthStencilTargetInfo.clear_depth = 1.0f;

		// std::cout << "Minimizing..." << std::endl;
		renderPass = SDL_BeginGPURenderPass(commandBuffer,
										   &colorTargetInfo,
										   1,
										    &depthStencilTargetInfo);
		if (!renderPass) {
			printf("Error beginning render pass: %s\n", SDL_GetError());
		}


	}

}


void Window::endFrame(){

	if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)) {
		//game
		SDL_EndGPURenderPass(renderPass);

		//ImGui
		SDL_GPUColorTargetInfo ImGuiColorTargetInfo{};

		ImGuiColorTargetInfo.texture = swapchainTexture;
		ImGuiColorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
		ImGuiColorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		ImGui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

		imguiRenderPass = SDL_BeginGPURenderPass(commandBuffer,&ImGuiColorTargetInfo,1,nullptr);

		if (imguiRenderPass == nullptr) {
			std::cerr << "Failed to begin imguiRenderPass" << SDL_GetError() << std::endl;
		}

		ImGui_ImplSDLGPU3_RenderDrawData(drawData,commandBuffer,imguiRenderPass);

		SDL_EndGPURenderPass(imguiRenderPass);

		// DO NOT TOUCH
		if(!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
			printf("Error submit command buffer: %s\n",SDL_GetError());
		}
	}


	mouseRel = {0,0};
}

SDL_GPUBuffer* Window::createBuffer(Uint32 size,SDL_GPUBufferUsageFlags usage ){
	SDL_GPUBufferCreateInfo info{};
	info.size = size;
	info.usage = usage;
    return SDL_CreateGPUBuffer(device,&info);
}

void Window::startCopyPass(){
    copyCommandBuffer = SDL_AcquireGPUCommandBuffer(device);
    if(copyCommandBuffer == NULL){
        printf("Error creating copy command buffer: %s\n",SDL_GetError());
    }
    copyPass = SDL_BeginGPUCopyPass(copyCommandBuffer);
    if(copyPass == NULL){
        printf("Error begain copy pass: %s\n",SDL_GetError());
    }
}

void Window::endCopyPass(){
    SDL_EndGPUCopyPass(copyPass);
    if(!SDL_SubmitGPUCommandBuffer(copyCommandBuffer)){
        printf("Error submit copy pass: %s\n",SDL_GetError());
    }
}

SDL_GPUTransferBuffer* Window::createTransferBuffer(Uint32 size){
	SDL_GPUTransferBufferCreateInfo info{};
	info.size = size;
	info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    return SDL_CreateGPUTransferBuffer(device,&info );
}


SDL_GPUTransferBufferLocation Window::createTransferBufferLocation(SDL_GPUTransferBuffer* transferBuffer,Uint32 offset){
    return {
        .transfer_buffer = transferBuffer,
        .offset = offset
    };
}

SDL_GPUBufferRegion Window::createBufferRegion(Uint32 size,SDL_GPUBuffer* buffer){
    return {
        .buffer = buffer,
        .size = size
    };
}

void Window::uploadBuffer(SDL_GPUTransferBufferLocation* transferBufferLocation, SDL_GPUBufferRegion* bufferRegion){
    SDL_UploadToGPUBuffer(copyPass, transferBufferLocation, bufferRegion, false);
}

SDL_GPUBufferBinding Window::createBufferBinding(SDL_GPUBuffer* buffer){
	return {
        .buffer = buffer,
        .offset = 0
    };
}


SDL_GPUTexture* Window::createTexture(SDL_Surface* surface){
	SDL_GPUTextureCreateInfo texture_create_info{};

	texture_create_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	texture_create_info.width = surface->w;
	texture_create_info.height = surface->h;
	texture_create_info.layer_count_or_depth = 1;
	texture_create_info.num_levels = 1;
	texture_create_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    return SDL_CreateGPUTexture(device, &texture_create_info);

}

void Window::createDepthStencilTexture(){
	SDL_ReleaseGPUTexture(device, depthTexture);
	depthTexture = nullptr;

	SDL_GetWindowSize(window,&width,&height);

	SDL_GPUTextureCreateInfo texture_create_info{};
	texture_create_info.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
	texture_create_info.width = width;
	texture_create_info.height = height;
	texture_create_info.layer_count_or_depth = 1;
	texture_create_info.num_levels = 1;
	texture_create_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	depthTexture =  SDL_CreateGPUTexture(device, &texture_create_info);
}

SDL_GPUSampler* Window::createSampler(){
	SDL_GPUSamplerCreateInfo sampler_create_info{};
	sampler_create_info.min_filter = SDL_GPU_FILTER_NEAREST;
	sampler_create_info.mag_filter = SDL_GPU_FILTER_NEAREST;
	sampler_create_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	sampler_create_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler_create_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler_create_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    return SDL_CreateGPUSampler(device,&sampler_create_info);
}

void Window::uploadTexture(SDL_GPUTextureTransferInfo* textureTransferInfo,SDL_GPUTextureRegion* textureRegion){
    SDL_UploadToGPUTexture(copyPass,textureTransferInfo,textureRegion,false);
}

void Window::cleanUp() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();

	// SDL_WaitGPUDeviceIdle(device); // make sure GPU is done using any textures/pipelines

	SDL_ReleaseGPUTexture(device, depthTexture);
	depthTexture = nullptr;

	SDL_ReleaseGPUShader(device, vertexShader);
	SDL_ReleaseGPUShader(device, fragmentTextureShader);
	SDL_ReleaseGPUShader(device, fragmentShapeTexture);

	SDL_ReleaseGPUGraphicsPipeline(device, texturePipeline);
	SDL_ReleaseGPUGraphicsPipeline(device, shapePipeline);
	SDL_ReleaseGPUSampler(device, sampler);

	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


void Window::keyboadInput(Event& event,float deltaTime) {
	vec2 move{};
#define e event.event
	if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
		if (e.key.scancode == SDL_SCANCODE_W) {
			move.y = 1;
		}

		if (e.key.scancode == SDL_SCANCODE_S) {
			move.y = -1;
		}

		if (e.key.scancode == SDL_SCANCODE_A) {
			move.x = -1;
		}

		if (e.key.scancode == SDL_SCANCODE_D) {
			move.x = 1;
		}
	}



	mouseRel = {e.motion.xrel, e.motion.yrel};

	vec2 mouseInput ;
	mat3 lookMat;
	if (e.type == SDL_EVENT_MOUSE_MOTION){
		mouseInput = mouseRel * lookSensitivity;

		yaw = std::clamp(yaw - mouseInput.x,-180.0f,180.0f);

		pitch = std::clamp(pitch - mouseInput.y, -89.0f,89.0f);
	}
	lookMat = mat3(yawPitchRoll(radians(yaw),radians(pitch),radians(0.0f)));
	vec3 forward{0,0,-1};
	vec3 right = {1,0,0};
	forward = lookMat * forward;
	right = lookMat * right;

	vec3 moveDir = forward * move.y + right * move.x;

	// std::cout << moveDir.x << " " << moveDir.y << " " << moveDir.z << std::endl;

	cameraPos += moveDir * moveSpeed * deltaTime;


	cameraTarget = cameraPos + forward;
	// SDL_WarpMouseInWindow(window, windowWidth/2, windowHeight/2);
}

void Window::ImGui() {
	ImGui::ShowDemoWindow();
}


SDL_Surface* loadImage(const char* imageFilename, int desiredChannels){
	char fullPath[256];
	SDL_Surface *result{};
	SDL_PixelFormat format{};

	SDL_snprintf(fullPath, sizeof(fullPath), "%s/%s", path, imageFilename);

	result = IMG_Load(fullPath);
	if (result == NULL){
		SDL_Log("Failed to load BMP: %s", SDL_GetError());
		return NULL;
	}

	if (desiredChannels == 4){
		format = SDL_PIXELFORMAT_ABGR8888;
	}
	else{
		SDL_assert(!"Unexpected desiredChannels");
		SDL_DestroySurface(result);
		return NULL;
	}
	if (result->format != format){
		SDL_Surface *next = SDL_ConvertSurface(result, format);
		SDL_DestroySurface(result);
		result = next;
	}

	return result;
}
