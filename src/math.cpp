#include <cmath>

#include "Math.hpp"
#include "Window.hpp"

void normalize(float &x, float &y, float &z,float windowWidth, float windowHeight) {
    x = x/windowWidth;
    y = y/windowHeight;
    z = z/1.0f;
}

//Un-normalized values + center
Model::Model(float x, float y, float extentX, float extentY,float windowWidth, float windowHeight) {
    vertices = {
        {x, y, 0.0f},
        {x + extentX, y, 0.0f},
        {x + extentX, y + extentY, 0.0f},
        {x, y + extentY, 0.0f}
    };

    for (auto& v : vertices) {
        normalize(v.x, v.y, v.z,windowWidth,windowHeight);
    }

    indices = {0, 1, 2, 2, 3, 0};
}


std::vector<ImageData> solidColor(Model* model, glm::vec3 color) {
    std::vector<ImageData> data(model->vertices.size());
    for (size_t i = 0; i < model->vertices.size(); i++) {
        data[i].color = color;
        data[i].texCoord = {0.0f, 0.0f}; // No texture coordinates for solid color
    }
    return data;
}

std::vector<ImageData> squareTexture(Model* model) {
    std::vector<ImageData> data(model->vertices.size());
    for (size_t i = 0; i < model->vertices.size(); i++) {
        data[i].color = {1.0f, 1.0f, 1.0f}; // White color for texture
        // Assign texture coordinates based on vertex position
        if (i == 0) data[i].texCoord = {0.0f, 0.0f}; // Bottom-left
        else if (i == 1) data[i].texCoord = {1.0f, 0.0f}; // Bottom-right
        else if (i == 2) data[i].texCoord = {1.0f, 1.0f}; // Top-right
        else if (i == 3) data[i].texCoord = {0.0f, 1.0f}; // Top-left
    }
    return data;
}