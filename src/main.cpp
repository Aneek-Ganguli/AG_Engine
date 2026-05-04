#include "Texture.hpp"
#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

void updateUniformBuffer(UBO& ubo, float time, float aspectRatio) {
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 1.0f));

    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 4.0f), // Moved camera to see the cube better
                          glm::vec3(0.0f, 0.0f, 0.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));

    ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);

    // VULKAN HIDDEN TRAP: GLM was designed for OpenGL where Y is UP.
    // Vulkan Y is DOWN. This flip is required unless you use a negative viewport.
    ubo.proj[1][1] *= -1;
}


int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);


    if (debug){std::cout << "Window variables " << window.width << " " << window.height << std::endl;}
    windowWidth = window.width;
    windowHeight = window.height;


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
    std::cout << "Window Global variables " << windowWidth << " " << windowHeight << std::endl;
    Entity entity(model,imageData,&text,&window);
    // Entity entity = Entity(&window,vertices,indices,&text);


        UBO ubo{};

    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        float aspect = windowWidth / (float)windowHeight;
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
