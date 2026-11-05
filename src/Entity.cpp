#include "Entity.hpp"

using namespace AG_EngineV2::Core;
using namespace AG_EngineV2;

Entity::Entity(Application *app, std::vector<VertexData> vertex, std::vector<uint16_t> indices, bool texture, Texture image, UniformBuffer ubo)
    :vertexBuffer(app->device.device, app->physicalDevice.physicalDevice ,app->commandPool[0].commandPool,&app->queue, vertex),
    indexBuffer(app->device.device, app->physicalDevice.physicalDevice ,app->commandPool[0].commandPool,&app->queue, indices),
    uniformBuffer(ubo)
{
    this->app = app;
    this->texture = texture;
    // descriptorWrites.push_back(uniformBuffer.descriptorWrite); // Binding 0
    indexCount = indices.size();

    if (texture) {
        this->image = image;
    }
}

void Entity::draw() {
    if (texture){
        app->bindPipeline(true);
        descriptorWrites = {
            uniformBuffer.descriptorWrite,
            image.textureWrite
        };
    }else{
        app->bindPipeline(false);
        descriptorWrites = {
            uniformBuffer.descriptorWrite, // Binding 0
        };
    }
    app->draw(&vertexBuffer, &indexBuffer, indexCount, descriptorWrites, true);
}

void Entity::destroy() {
    vertexBuffer.destroy(app->device.device);
    indexBuffer.destroy(app->device.device);
    uniformBuffer.destroy(app->device.device);
    if (texture) {
        image.destroy(app->device.device);
    }
}
