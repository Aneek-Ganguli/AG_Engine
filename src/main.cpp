#include "Texture.hpp"
#include  <iostream>
#include <Window.hpp>
#include <GLFW/glfw3.h>

#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>


int main() {
    std::cout << "Hello World!" << std::endl;

    Window window("im a Tau Cetian",800,600);


    std::cout << "Window variables " << window.width << " " << window.height << std::endl;
    windowWidth = window.width;
    windowHeight = window.height;


    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    std::vector<ImageData> imageData{};
    Model model;
    for (int i = 0; i < vertices.size(); i++) {
        model.vertices.push_back(vertices[i].pos);
        imageData.push_back({vertices[i].color, vertices[i].texCoord});
    }
    model.indices = indices;

    Texture text("res/resource.jpg",&window);
    // Texture text{};
    std::cout << "Window Global variables " << windowWidth << " " << windowHeight << std::endl;
    Entity entity(model,imageData,&text,&window);
    // Entity entity = Entity(&window,vertices,indices,&text);



    while (window.isWindowOpen()) {
        glfwPollEvents();

        window.startFrame();

        UBO ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Use actual window dimensions from the Window class
        float aspect = windowWidth / (float)windowHeight;
        ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
        entity.updateUniformBuffer(&ubo,frameIndex);
        entity.draw(&window);

        window.endFrame();
    }

    text.cleanUp(&window);
    entity.cleanUp(&window);
    window.cleanUp();
    return 0;
}
