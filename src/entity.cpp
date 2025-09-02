#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Window.hpp"
#include "Entity.hpp"
#include "VertexData.hpp"

Entity::Entity(std::vector<VertexData> p_vertexData, std::vector<Uint32> p_indices, Transform3D p_transform ,
    const char* p_fileName,Window* window) {
    // Reset everything
    // *e = (struct Entity){0};

    const Uint32 vertexSize = p_vertexData.size() * sizeof(VertexData);
    const Uint32 indexSize  = p_indices.size() * sizeof(Uint32);

    verticiesCount = p_vertexData.size();
    indiciesCount  = p_indices.size();

    // --- GPU buffers ---
    vertexBuffer = createBuffer(vertexSize, SDL_GPU_BUFFERUSAGE_VERTEX, window);
    indexBuffer  = createBuffer(indexSize,  SDL_GPU_BUFFERUSAGE_INDEX,  window);
    if (!vertexBuffer || !indexBuffer) {
        printf("Error creating vertex/index buffer: %s\n", SDL_GetError());
        return;
    }

    // --- Staging for vertex+index (one big upload) ---
    transferBuffer = createTransferBuffer(vertexSize + indexSize, window);
    if (!transferBuffer) {
        printf("Error creating transfer buffer: %s\n", SDL_GetError());
        return;
    }

    transferMem = SDL_MapGPUTransferBuffer(window->device, transferBuffer, false);
    if (!transferMem) {
        printf("Error mapping transfer buffer: %s\n", SDL_GetError());
        return;
    }
    memcpy(transferMem, p_vertexData.data(), vertexSize);
    memcpy((char*)transferMem + vertexSize, p_indices.data(), indexSize);
    SDL_UnmapGPUTransferBuffer(window->device, transferBuffer);

    vertexTransferBufferLocation = createTransferBufferLocation(transferBuffer, 0);
    indexTransferBufferLocation  = createTransferBufferLocation(transferBuffer, vertexSize);

    vertexBufferRegion = createBufferRegion(vertexSize, vertexBuffer);
    indexBufferRegion  = createBufferRegion(indexSize,  indexBuffer);

    // --- Texture load + GPU texture ---

    texture1.create(p_fileName, window);
    // --- Perform uploads (must be inside an active copy pass) ---
    if (!window->copyPass) {
        printf("WARNING: upload called without active copy pass!\n");
    }



    uploadBuffer(&vertexTransferBufferLocation, &vertexBufferRegion, window);
    uploadBuffer(&indexTransferBufferLocation,  &indexBufferRegion,  window);
    texture1.upload(window);


    // --- Bindings for draw ---
    vertexBufferBinding = createBufferBinding(vertexBuffer);
    indexBufferBinding  = createBufferBinding(indexBuffer);

    transform = p_transform;
}


// void print_mat4(mat4 m) {
//     for (int i = 0; i < 4; i++) {
//         printf("| ");
//         for (int j = 0; j < 4; j++) {
//             printf("%8.3f ", m[i][j]);
//         }
//         printf("|\n");
//     }
//     printf("\n");
// }

void Entity::draw(Window* window) {
    // glm_mat4_identity(transform.M);
    transform.M = glm::mat4(1.0f);
    // translate (glm::translate returns a new matrix)
    transform.M = glm::translate(transform.M, transform.position);
    // scale
    transform.M = glm::scale(transform.M, glm::vec3(3.0f, 1.0f, 1.0f));
    // multiply P * M -> MVP
    transform.transform.mvp = window->P * transform.M;

    SDL_BindGPUVertexBuffers(window->renderPass, 0, &vertexBufferBinding, 1);
    SDL_BindGPUIndexBuffer(window->renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    SDL_PushGPUVertexUniformData(window->commandBuffer, 0, &transform.transform, sizeof(transform.transform));

    texture1.bind(window, 2, 0); // → set=2, binding=0

    if (!window->renderPass) {
        std::cout << "Error creating renderpass" << SDL_GetError()<< std::endl;
    }

    SDL_DrawGPUIndexedPrimitives(window->renderPass, (Uint32)indiciesCount, 1, 0, 0, 0);
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
        SDL_ReleaseGPUBuffer(window->device, vertexBuffer);
        vertexBuffer = NULL;
    }
    if (indexBuffer) {
        SDL_ReleaseGPUBuffer(window->device, indexBuffer);
        indexBuffer = NULL;
    }
    if (transferBuffer) {
        SDL_ReleaseGPUTransferBuffer(window->device, transferBuffer);
        transferBuffer = NULL;
    }
    texture1.destroy(window);
    // texture2.destroy(window);
}

// void setScale(Entity* e,vec3 scale){
//     glm_vec3_copy(scale,scale);
// }
