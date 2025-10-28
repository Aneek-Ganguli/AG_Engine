#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Window.hpp"
#include "VertexData.hpp"

using namespace glm;
namespace AG_Engine {
    struct  Transform{
    public:
        //Constructor
        Transform(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle);
        Transform(vec2 p_position, vec2 p_rotation, vec2 p_scale,float p_rotateAngle);
        Transform():position(NULL),M(NULL),scale(NULL),rotate(NULL),rotateAngle(NULL),transform(NULL){}

        //Transform Data
        struct UBO* getUBOData();
        Uint32 getUBOSize();
        void translate(glm::mat4 view, glm::mat4 projection,float deltaTime);

        //Collision
        bool checkCollisionX(const Transform& otherObject);
        bool checkCollisionY(const Transform& otherObject);
        bool checkCollisionZ(const Transform& otherObject);
        bool checkCollision(const Transform& otherObject);

    private:
        vec3 position{},scale{},rotate{};
        mat4 M{};
        float rotateAngle{};
        struct UBO transform{};
    };

}


// ok so i want to get the z and calculate thhe maximum x,y and clamp it
