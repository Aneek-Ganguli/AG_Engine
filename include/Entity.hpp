#pragma once
#include <stdio.h>
#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>


#include "ModelData.hpp"
#include "Window.hpp"
#include "VertexData.hpp"
#include "Transform.hpp"
#include "Texture.hpp"


namespace AG_Engine{
    class Entity{
    public:
        Entity(ModelData p_modelData, Transform p_transform,
               Texture& p_texture,Window *window);

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
        Texture texture1{{}};

        std::vector<vec2> collisionVerticies;

        //Transform
        Transform transform{};

        std::vector<VertexData> vertexData{};

        //Misc
        int verticiesCount{}, indiciesCount{};
    } ;
}

