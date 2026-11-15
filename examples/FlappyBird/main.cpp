#include <iostream>
#include <Application.hpp>
#include <Entity.hpp>

#include <glm/ext/matrix_transform.hpp>

using namespace AG_EngineV2;

int main() {

        Application app("Appey",0,1,true);


        std::vector<VertexData> vertices = {
            // Front Face (Red)
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        };

        std::vector<uint16_t> indices = {
            0,  1,  2,  2,  3,  0,  // Front
        };


        UBO mvp;
        mvp.projection = glm::mat4(1.0f);
        mvp.view = glm::mat4(1.0f);
        // mvp.view = glm::perspective()
        mvp.model = glm::mat4(1.0f);
        mvp.model = glm::scale(mvp.model, glm::vec3(.5f, .5f, .5f));
        mvp.uvOffset = glm::vec2(0.0f);
        mvp.uvScale = glm::vec2(1.0f);


        UniformBuffer uniformBuffer(app.device.device, app.physicalDevice.physicalDevice,sizeof(UBO), &mvp, 0,0,0,vk::DescriptorType::eUniformBuffer);
        uint32_t index = 95;
        UniformBuffer imageIndexUniformBuffer(app.device.device, app.physicalDevice.physicalDevice,sizeof(uint32_t), &index, 0,0,1,vk::DescriptorType::eUniformBuffer);
        Texture texture(app.device.device, &app.queue, app.physicalDevice.physicalDevice, "resource.jpg",false);
        Entity entity(&app, vertices, indices, index, true, texture, imageIndexUniformBuffer , uniformBuffer);

        app.setClearValue(vk::ClearColorValue(std::array<float, 4>({0.0f, 1.0f, 1.0f, 1.0f})));

        // Texture pipeTexture()
        std::vector<UniformBuffer> mvpUBO(5);
        // std::vector<UniformBuffer> imageIndexUBO(5);
        std::vector<Entity> pipes(5);
            UBO newUBO[5];
        for (int i = 0; i < 5; i++) {
            newUBO[i].projection = glm::mat4(1.0f);
            newUBO[i].view = glm::mat4(1.0f);
            newUBO[i].model = glm::mat4(1.0f);
            newUBO[i].uvScale = glm::vec2(1.0f);
            newUBO[i].uvOffset = glm::vec2(0.0f);

            float x = float(i)/float(pipes.size());
            newUBO[i].model = glm::translate(newUBO[i].model, glm::vec3(-2.0f*x, .5f, 0.0f));
            newUBO[i].model  = glm::scale(newUBO[i].model, glm::vec3(.25,1,0.5f));
                // newUBO[i].model = glm::translate(newUBO[i].model, glm::vec3(-2.0f*x, .5f, 0.0f));
            mvpUBO[i] = UniformBuffer(app.device.device, app.physicalDevice.physicalDevice,sizeof(UBO), &newUBO[i], 0,0,0,vk::DescriptorType::eUniformBuffer);
            pipes[i] = Entity(&app, vertices, indices, index, true, texture, imageIndexUniformBuffer , mvpUBO[i]);
        }


        float lastFrameTime = 0.0f;
        bool keyLock = false;
        std::vector<vk::WriteDescriptorSet> descriptorWrite = { uniformBuffer.descriptorWrite};
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3( 0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 0.25, 0.5f));
        bool open = app.isWindowOpen();
        while (open) {
            app.startFrame();

            float currentFrameTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;

            glm::vec3 moveInput(0.0f);
            if (glfwGetMouseButton(app.window.window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
                if (!keyLock) {
                    moveInput.y -= 40;
                    keyLock = true;
                }
            }
            else{
                keyLock = false;
                moveInput.y +=2;
            }
            UBO updatedUBO;
            model = glm::translate(model, moveInput*deltaTime);
            updatedUBO.model = model;
            updatedUBO.view  = mvp.view;
            updatedUBO.projection  = mvp.projection;
            updatedUBO.uvOffset    = mvp.uvOffset;
            updatedUBO.uvScale     = mvp.uvScale;
            entity.indexUBO.updateData(&entity.index);
            entity.uniformBuffer.updateData(&updatedUBO);
            entity.draw();
            //
            if (updatedUBO.model[3][1] > 1.0f) {
                app.clearValue = vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
                std::cout << "YOU LOSE!!!!!!!!!\n HAHAHAHAHAHAHA";
                break;
            }

            for (int i = 0; i < pipes.size(); i++) {
                // Generate a random number between 0 and 100
                // float x = (rand() % app.window.width)/(float)app.window.width;

                // srand(time1^19);

                // float y = (rand() % app.window.height)/(float)app.window.height;
                float x = float(i)/float(pipes.size());
                // if (i==0) {
                //     newUBO[i].model  = glm::translate(newUBO[i].model, glm::vec3(2.5f*deltaTime*(6.0f/10.0f),0.0,0.0));
                // }
                // else {
                //     newUBO[i].model  = glm::translate(newUBO[i].model, glm::vec3((2.5f*deltaTime*x),0.0,0.0));
                // }
                float speed = 2.5f;
                switch (i) {
                    case 0: newUBO[i].model[3][0] += speed * deltaTime * 0.9f; break;
                    case 1: newUBO[i].model[3][0] += speed * deltaTime * 0.4f; break;
                    case 2: newUBO[i].model[3][0] += speed * deltaTime * 2.6f; break;
                    case 3: newUBO[i].model[3][0] += speed * deltaTime * 1.5f; break;
                    case 4: newUBO[i].model[3][0] += speed * deltaTime * 1.0f; break;
                }
                if (newUBO[i].model[3][0] > 1.5f) {
                    newUBO[i].model  = glm::translate(newUBO[i].model, glm::vec3(-10.0f,0.0,0.0));
                }
                if (updatedUBO.model[3][0] + 0.25f > newUBO[i].model[3][0] && updatedUBO.model[3][0] - 0.25f < newUBO[i].model[3][0]) {
                    if (updatedUBO.model[3][1] + 0.5f > newUBO[i].model[3][1] && updatedUBO.model[3][1] - 0.5f < newUBO[i].model[3][1]) {
                        app.clearValue = vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
                        std::cout << "YOU LOSE!!!!!!!!!\n HAHAHAHAHAHAHA";
                        open = false;
                    }
                }
                std::cout << i << " " << newUBO[i].model[3].x << " " << newUBO[i].model[3].y << " "<< newUBO[i].model[3].z<< std::endl;
                pipes[i].indexUBO.updateData(&index);
                pipes[i].uniformBuffer.updateData(&newUBO[i]);
                pipes[i].draw();
            }
            app.endFrame();


        }
        app.waitIdle();
        for (int i = 0; i < pipes.size(); i++) {
            pipes[i].vertexBuffer.destroy(app.device.device);
            pipes[i].indexBuffer.destroy(app.device.device);
            pipes[i].uniformBuffer.destroy(app.device.device);
        }
        entity.destroy();
        app.destroy();
    }