#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Window.hpp"
#include "VertexData.hpp"

using namespace glm;
namespace AG_Engine {
    struct  Transform3D{
    public:
        Transform3D(vec3 p_position, vec3 p_rotation, vec3 p_scale,float p_rotateAngle);
        Transform3D():position(NULL),M(NULL),scale(NULL),rotate(NULL),rotateAngle(NULL),transform(NULL){}
        UBO* getUBOData();
        Uint32 getUBOSize();
        void translate(glm::mat4 view, glm::mat4 projection,float deltaTime);

        bool checkCollisionX(const Transform3D& otherObject);

        bool checkCollisionY(const Transform3D& otherObject);

        bool checkCollisionZ(const Transform3D& otherObject);

        bool checkCollision(const Transform3D& otherObject);

    private:
        vec3 position{},scale{},rotate{};
        mat4 M{};
        float rotateAngle{};
        UBO transform{};
    };


}


// ok so i want to get the z and calculate thhe maximum x,y and clamp it
