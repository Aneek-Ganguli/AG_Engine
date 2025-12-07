#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
// #define CGLTF_IMPLEMENTATION
// #include <cgltf/cgltf.h>

using namespace glm;

namespace AG_Engine {
    struct VertexData {
        alignas(16) glm::vec3 position;
        glm::vec2 texCoord[8];
    };

    struct UBO {
        alignas(16) glm::mat4 mvp;
    };

    struct ColorUniform {
        alignas(16) glm::vec4 color;
    };
}

