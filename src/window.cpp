#include <iostream>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlgpu3.h>

#include "Window.hpp"
#include "VertexData.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"


Window::Window(const char* title,int width,int height):width(width),height(height){
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
	if(device == NULL){
		printf("Error creating gpu device: %s: \n",SDL_GetError());
	}

	SDL_ClaimWindowForGPUDevice(device,window);

	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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


	// view = glm::lookAt(cameraPosition,cameraTarget,{0,1,0});
}

SDL_GPUShader* Window::loadShader(
	const char* shaderFilename,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount) {
	// Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage;
	if (SDL_strstr(shaderFilename, ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(shaderFilename, ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage!");
		return NULL;
	}

	char fullPath[256];
	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	const char *entrypoint;

	if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
		SDL_snprintf(fullPath, sizeof(fullPath), "%s.spv", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "%s/%s.msl", "../bin/shader", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "%s/%s.dixil","../bin/shader", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	} else {
		SDL_Log("%s", "Unrecognized backend shader format!");
		return NULL;
	}

	size_t codeSize;
	void* code = SDL_LoadFile(fullPath, &codeSize);
	if (code == NULL)
	{
		SDL_Log("Failed to load shader from disk! %s", fullPath);
		return NULL;
	}

	SDL_GPUShaderCreateInfo shaderInfo{};
	shaderInfo.code = static_cast<const Uint8*>(code);
	shaderInfo.code_size = codeSize;
	shaderInfo.entrypoint = entrypoint;
	shaderInfo.format = format;
	shaderInfo.stage = stage;
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

	SDL_free(code);
	return shader;
}



void Window::startFrame(){
	// ImGui_ImplSDLGPU3_NewFrame();
	// ImGui_ImplSDL3_NewFrame();
	// ImGui::NewFrame();

    commandBuffer = SDL_AcquireGPUCommandBuffer(device);
    if(commandBuffer == NULL){
        printf("Error aquire command buffer: %s: \n",SDL_GetError());
    }
    
    if(!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer,window,&swapchainTexture,NULL,NULL)){
        printf("Error acquire swapchain texture: %s\n",SDL_GetError());
    }

	depthTexture = createDepthStencilTexture();
	if (!depthTexture) {
		printf("Error creating depth texture: %s\n", SDL_GetError());
	}

	SDL_GPUColorTargetInfo colorTargetInfo{};
	colorTargetInfo.texture = swapchainTexture;
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	colorTargetInfo.clear_color = {0.0f,1.0f,1.0f,1.0f};

	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
	depthStencilTargetInfo.texture = depthTexture;
	depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
	depthStencilTargetInfo.clear_depth = 1.0f;

    renderPass =  SDL_BeginGPURenderPass(commandBuffer,
        &colorTargetInfo,
        1,
        &depthStencilTargetInfo
    );
    if(renderPass == NULL){
        printf("Error begain render pass: %s \n",SDL_GetError());
    }

    SDL_BindGPUGraphicsPipeline(renderPass,pipeline);


}

void Window::endFrame(){

	// ImGui::Render();
	// ImDrawData* draw_data = ImGui::GetDrawData();
	// ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, commandBuffer);
	//
	// ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer, renderPass);

	SDL_EndGPURenderPass(renderPass);
    if(!SDL_SubmitGPUCommandBuffer(commandBuffer)){
        printf("Error submit command buffer: %s\n",SDL_GetError());
    }
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

void print_mat4(mat4 m) {
	for (int i = 0; i < 4; i++) {
		printf("| ");
		for (int j = 0; j < 4; j++) {
			printf("%8.3f ", m[i][j]);
		}
		printf("|\n");
	}
	printf("\n");
}

void Window::createGraphicsPipeline() {
	vertexShader = loadShader("res/shader/shader.vert", 0, 1, 0, 0);

	fragmentShader = loadShader("res/shader/shader.frag", 1, 0, 0, 0);
	SDL_GPUVertexBufferDescription vertexBufferDescription{};
	vertexBufferDescription.slot = 0;
	vertexBufferDescription.pitch = sizeof(VertexData);
	vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute vertexAttributes[3]{};
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
	vertexAttributes[2] = {
		.location = 2,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
		.offset = offsetof(struct VertexData,color),
	};

	SDL_GPUVertexInputState vertexInput{};
	vertexInput.num_vertex_buffers = 1;
	vertexInput.num_vertex_attributes = 3;
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
	graphics_pipeline_create_info.fragment_shader = fragmentShader;
	graphics_pipeline_create_info.depth_stencil_state = depthStencilState;

    pipeline = SDL_CreateGPUGraphicsPipeline(device,&graphics_pipeline_create_info);
    if(pipeline == NULL){
        printf("Erro graphics pipeline :%s\n",SDL_GetError());
    }

    sampler = createGPUSampler();
    // fov = Fov;
    windowWidth = width;
    windowHeight = height;

	// print_mat4(projection);
	std::cout << windowWidth << " " << windowHeight << std::endl;
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

SDL_GPUTexture* Window::createDepthStencilTexture(){
	SDL_GetWindowSize(window,&width,&height);
	SDL_GPUTextureCreateInfo texture_create_info{};
	texture_create_info.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
	texture_create_info.width = width;
	texture_create_info.height = height;
	texture_create_info.layer_count_or_depth = 1;
	texture_create_info.num_levels = 1;
	texture_create_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	return SDL_CreateGPUTexture(device, &texture_create_info);
}

SDL_GPUSampler* Window::createGPUSampler(){
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

void Window::cleanUp(){
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();
    SDL_ReleaseGPUShader(device,vertexShader);
    SDL_ReleaseGPUShader(device,fragmentShader);
    SDL_ReleaseGPUGraphicsPipeline(device,pipeline);
    SDL_ReleaseGPUSampler(device,sampler);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::createPerspective(float p_fov) {
	fov = p_fov;

	// glm_perspective(fov, (float)width/height, 0.1f, 1000.0f, P);
	projection = perspective(glm::radians(fov),(float)width/height,0.0001f,1000.0f);
}


