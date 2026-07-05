#include "Texture.hpp"
#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);

    std::vector<glm::vec3> vertices = {
        // Front face (z = 1.0f)
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f},
        // Back face (z = -1.0f)
        {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f},
        // Top face (y = 1.0f)
        {-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f, -1.0f},
        // Bottom face (y = -1.0f)
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f},
        // Right face (x = 1.0f)
         {1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f},
        // Left face (x = -1.0f)
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f, -1.0f}
    };

    std::vector<uint16_t> indices = {
        0,  1,  2,   2,  3,  0,  // Front
        4,  5,  6,   6,  7,  4,  // Back
        8,  9,  10,  10, 11, 8,  // Top
        12, 13, 14,  14, 15, 12, // Bottom
        16, 17, 18,  18, 19, 16, // Right
        20, 21, 22,  22, 23, 20  // Left
    };
    std::vector<ImageData> imageData = {
        // Front
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // Back
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // Top
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // Bottom
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // Right
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        // Left
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };
    Model model;
    model.vertices = vertices;
    model.indices = indices;


    Texture text("res/resource.jpg",&window);
    // Texture text{};
    Entity entity(model,imageData,&text,&window);
    // Entity entity = Entity(&window,vertices,indices,&text);


        UBO ubo{};

    while (window.isWindowOpen()) {
        window.pollEvents();

        window.startFrame();

        float aspect = window.width / (float)window.height;
        updateUniformBuffer(ubo, static_cast<float>(glfwGetTime()), aspect);

        entity.updateUniformBuffer(&ubo,sizeof(ubo),frameIndex);
        entity.draw(&window);

        window.endFrame();
    }

    text.cleanUp(&window);
    entity.cleanUp(&window);
    window.cleanUp();
    return 0;
}
