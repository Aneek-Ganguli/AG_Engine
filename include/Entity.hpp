#pragma once
#include "Application.hpp"
#include "Texture.hpp"
#include "UniformBuffer.hpp"

using namespace AG_EngineV2::Core;
using namespace AG_EngineV2;

namespace AG_EngineV2 {
    struct UBO {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec2 uvOffset;
        glm::vec2 uvScale;
        float ambient = 0.02;
    };
    class Entity {
    public:
        Application* app;
        Entity(Application *app, std::vector<VertexData> vertex, std::vector<uint16_t> indices, uint32_t _index, bool texture, Texture _image, UniformBuffer indexUBO, UniformBuffer  ubo);
        Entity() = default;
        VertexBuffer vertexBuffer;
        IndexBuffer indexBuffer;
        UniformBuffer uniformBuffer{};
        UniformBuffer indexUBO{};
        Texture image{};
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        size_t indexCount;
        void draw();
        void destroy();
        bool texture;
        uint32_t index;
    };
}