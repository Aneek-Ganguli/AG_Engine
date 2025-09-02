#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
// #define CGLTF_IMPLEMENTATION
// #include <cgltf/cgltf.h>

using namespace glm;

struct VertexData {
    glm::vec3 position;
    glm::vec2 texCoord;
    SDL_FColor color;
};



typedef struct UBO {
	glm::mat4 mvp;
} UBO;

void perspective_bounds(float fovy, float aspect, float z,
                        float *xmin, float *xmax,
                        float *ymin, float *ymax);

// vec2* normalize(float x, float y);
