#include <cmath>

#include "Math.hpp"
#include "Window.hpp"

void normalize(float &x, float &y, float &z) {
    x = x/windowWidth;
    y = y/windowHeight;
    z = z/1.0f;
}

Model::Model(float x, float y, float extentX, float extentY) {
    vertices = {
        {x, y, 0.0f},
        {x + extentX, y, 0.0f},
        {x + extentX, y + extentY, 0.0f},
        {x, y + extentY, 0.0f}
    };

    for (auto& v : vertices) {
        normalize(v.x, v.y, v.z);
    }

    indices = {0, 1, 2, 2, 3, 0};
}
