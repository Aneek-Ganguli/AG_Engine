#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Window.hpp"
#include "Entity.hpp"
#include "VertexData.hpp"

using namespace AG_Engine;

Entity::Entity(std::vector<VertexData> p_vertexData, std::vector<Uint32> p_indices, Transform3D p_transform ,
    const char* p_fileName,Window* window) {
    // Reset everything
    // *e = (struct Entity){0};

    const Uint32 vertexSize = p_vertexData.size() * sizeof(VertexData);
    const Uint32 indexSize  = p_indices.size() * sizeof(Uint32);

    verticiesCount = p_vertexData.size();
    indiciesCount  = p_indices.size();

    // --- GPU buffers ---
    vertexBuffer = window->createBuffer(vertexSize, SDL_GPU_BUFFERUSAGE_VERTEX);
    indexBuffer  = window->createBuffer(indexSize,  SDL_GPU_BUFFERUSAGE_INDEX);
    if (!vertexBuffer || !indexBuffer) {
        printf("Error creating vertex/index buffer: %s\n", SDL_GetError());
        return;
    }

    // --- Staging for vertex+index (one big upload) ---
    transferBuffer = window->createTransferBuffer(vertexSize + indexSize);
    if (!transferBuffer) {
        printf("Error creating transfer buffer: %s\n", SDL_GetError());
        return;
    }

    transferMem = SDL_MapGPUTransferBuffer(window->getGPUDevice(), transferBuffer, false);
    if (!transferMem) {
        printf("Error mapping transfer buffer: %s\n", SDL_GetError());
        return;
    }
    memcpy(transferMem, p_vertexData.data(), vertexSize);
    memcpy((char*)transferMem + vertexSize, p_indices.data(), indexSize);
    SDL_UnmapGPUTransferBuffer(window->getGPUDevice(), transferBuffer);

    vertexTransferBufferLocation =  window->createTransferBufferLocation(transferBuffer, 0);
    indexTransferBufferLocation  = window->createTransferBufferLocation(transferBuffer, vertexSize);

    vertexBufferRegion = window->createBufferRegion(vertexSize, vertexBuffer);
    indexBufferRegion  =  window->createBufferRegion(indexSize,  indexBuffer);

    // --- Texture load + GPU texture ---

    texture1.create(p_fileName, window);

    window->uploadBuffer(&vertexTransferBufferLocation, &vertexBufferRegion);
    window->uploadBuffer(&indexTransferBufferLocation,  &indexBufferRegion);
    texture1.upload(window);


    // --- Bindings for draw ---
    vertexBufferBinding =  window->createBufferBinding(vertexBuffer);
    indexBufferBinding  =  window->createBufferBinding(indexBuffer);

    transform = p_transform;
}


void Entity::draw(Window* window,float deltaTime) {

    if (!(SDL_GetWindowFlags(window->getWindow()) & SDL_WINDOW_MINIMIZED)){
        transform.translate(window->view, window->projection,deltaTime);

        SDL_BindGPUVertexBuffers(window->getRenderPass(), 0, &vertexBufferBinding, 1);
        SDL_BindGPUIndexBuffer(window->getRenderPass(), &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

        SDL_PushGPUVertexUniformData(window->getCommandBuffer(), 0, transform.getUBOData(),
            transform.getUBOSize());

        texture1.bind(window, 0, 1); // → set=2, binding=0



        SDL_DrawGPUIndexedPrimitives(window->getRenderPass(), (Uint32)indiciesCount, 1, 0, 0, 0);
    }
}


std::vector<VertexData> loadModel(const std::string& path, std::vector<Uint32>& indices) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<VertexData> vertices;

    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto& mesh = scene->mMeshes[i];

        // Process vertices
        for (size_t j = 0; j < mesh->mNumVertices; ++j) {
            VertexData vertex;
            vertex.position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
            vertex.texCoord = mesh->mTextureCoords[0] ? vec2{ mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y } : vec2{ 0.0f, 0.0f };
            vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Default white color
            vertices.push_back(vertex);
        }

        // Process indices
        for (size_t j = 0; j < mesh->mNumFaces; ++j) {
            const auto& face = mesh->mFaces[j];
            for (size_t k = 0; k < face.mNumIndices; ++k) {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    return vertices;
}



// void createEntityWithModel(const char* modelFileName,const char* textureFileName,vec3 scale,vec3 position,Window* window, Entity* entity){
    // std::vector<Uint32> indices;
    // std::vector<VertexData> vertexData = loadModel(modelFileName,indices);
    // createEntity(vertexData,indices,textureFileName,position,scale,window,entity);
// }

void Entity::destroy(Window* window){
    if (vertexBuffer) {
        SDL_ReleaseGPUBuffer(window->getGPUDevice(), vertexBuffer);
        vertexBuffer = NULL;
    }
    if (indexBuffer) {
        SDL_ReleaseGPUBuffer(window->getGPUDevice(), indexBuffer);
        indexBuffer = NULL;
    }
    if (transferBuffer) {
        SDL_ReleaseGPUTransferBuffer(window->getGPUDevice(), transferBuffer);
        transferBuffer = NULL;
    }
    texture1.destroy(window);
    // texture2.destroy(window);
}

