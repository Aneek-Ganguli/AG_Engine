#pragma once
#include  <glm/glm.hpp>

struct UBO {
    UBO(){
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
        proj = glm::mat4(1.0f);
    }
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};