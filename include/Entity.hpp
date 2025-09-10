#pragma once
#include <stdio.h>
#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>


#include "Window.hpp"
#include "VertexData.hpp"
#include "Component.hpp"

class Entity{
public:
    Entity(std::vector<VertexData> p_vertexData, std::vector<Uint32> p_indices,Transform3D p_transform,
        const char* p_fileName,Window* window);

    void draw(Window *window, float deltaTime);

    void destroy(Window* window);

private:

    //vertex
    SDL_GPUBuffer *vertexBuffer{};
    SDL_GPUTransferBufferLocation vertexTransferBufferLocation{};
    SDL_GPUBufferRegion vertexBufferRegion{};
    SDL_GPUBufferBinding vertexBufferBinding{};

    //index
    SDL_GPUBuffer *indexBuffer{};
    SDL_GPUTransferBufferLocation indexTransferBufferLocation{};
    SDL_GPUBufferRegion indexBufferRegion{};
    SDL_GPUBufferBinding indexBufferBinding{};

    //index + vertex
    SDL_GPUTransferBuffer *transferBuffer{};
    void *transferMem{};

    //texture
    Texture texture1{};


    //Transform
    Transform3D transform{};

    //Misc
    int verticiesCount{}, indiciesCount{};
} ;

// void createEntity(std::vector<VertexData> vertexData, std::vector<Uint32> indicies, const char* fileName,
    // vec3 position,vec3 scale,struct Window *window, struct Entity *e);


// void drawEntity(struct Window *window, Entity *e);

// void destroyEntity(Entity* e,Window* window);

// void setScale(Entity* e, vec3 scale);

std::vector<VertexData> loadModel(const std::string& path, std::vector<Uint32>& indices);
