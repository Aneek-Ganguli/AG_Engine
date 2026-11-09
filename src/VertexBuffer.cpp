#include "VertexBuffer.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include "core/Queue.hpp"
#include "Buffer.hpp"
#include "core/CommandBuffer.hpp"
#include "core/CommandPool.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace AG_EngineV2;
using namespace AG_EngineV2::Core;

void AG_EngineV2::copyBuffer(vk::Buffer srcBuffer,vk::Buffer dstBuffer, vk::DeviceSize size, vk::Device device,Queue* queue) {
    CommandPool copyCommandPool(device,queue->graphicsQueueFamilyIndex);
    CommandBuffer copyCommandBuffer(device,copyCommandPool.commandPool);

    copyCommandBuffer.begin();
    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;

    copyCommandBuffer.commandBuffer[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    copyCommandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer.commandBuffer[0];

    if(queue->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit copy command buffer");
    }

    queue->graphicsQueue.waitIdle();

    device.resetCommandPool(copyCommandPool.commandPool, {});
    device.freeCommandBuffers(copyCommandPool.commandPool, 1, &copyCommandBuffer.commandBuffer[0]);
    device.destroyCommandPool(copyCommandPool.commandPool);
}

AG_EngineV2::VertexBuffer::VertexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, Queue* queue, const std::vector<VertexData> vertices) {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

     stagingBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
    | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data;
    if (device.mapMemory(stagingBuffer.memory, 0, bufferSize,{},&data) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to map vertex buffer memory!") ;
    }
    memcpy(data, vertices.data(), (size_t) bufferSize);
    device.unmapMemory(stagingBuffer.memory);


     vertexBuffer = Buffer(device,physicalDevice,bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    copyBuffer(stagingBuffer.buffer,vertexBuffer.buffer,bufferSize,device,queue);

    stagingBuffer.destroy(device);

    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(vertices[0]);
    bindingDesc.inputRate = vk::VertexInputRate::eVertex;
    bindingDesc.divisor = 1;

    attribDesc[0].location = 0;
    attribDesc[0].binding = 0;
    attribDesc[0].format = vk::Format::eR32G32B32Sfloat;
    attribDesc[0].offset = offsetof(VertexData,vertices);

    attribDesc[1].location = 1;
    attribDesc[1].binding = 0;
    attribDesc[1].format = vk::Format::eR32G32B32Sfloat;
    attribDesc[1].offset = offsetof(VertexData,color);

    attribDesc[2].location = 2;
    attribDesc[2].binding = 0;
    attribDesc[2].format = vk::Format::eR32G32Sfloat;
    attribDesc[2].offset = offsetof(VertexData,texCoord) ;

    attribDesc[3].location = 3;
    attribDesc[3].binding = 0;
    attribDesc[3].format = vk::Format::eR32G32B32Sfloat;
    attribDesc[3].offset = offsetof(VertexData,normal);
}

void VertexBuffer::destroy(vk::Device device) {
    vertexBuffer.destroy(device);
}

void AG_EngineV2::generateNormals(std::vector<VertexData>& vertices, const std::vector<uint16_t>& indices) {
    // Reset all normals to zero
    for (auto& v : vertices) {
        v.normal = glm::vec3(0.0f);
    }

    // Accumulate face normals for each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        uint16_t idx0 = indices[i];
        uint16_t idx1 = indices[i + 1];
        uint16_t idx2 = indices[i + 2];

        glm::vec3 p0 = vertices[idx0].vertices;
        glm::vec3 p1 = vertices[idx1].vertices;
        glm::vec3 p2 = vertices[idx2].vertices;

        // Cross product of two triangle edges
        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec3 faceNormal = glm::cross(edge1, edge2);

        // Accumulate (area-weighted)
        vertices[idx0].normal += faceNormal;
        vertices[idx1].normal += faceNormal;
        vertices[idx2].normal += faceNormal;
    }

    // Normalize the accumulated vectors
    for (auto& v : vertices) {
        float len = glm::length(v.normal);
        if (len > 0.0f) {
            v.normal = glm::normalize(v.normal);
        } else {
            v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Fallback
        }
    }
}

std::vector<VertexData> AG_EngineV2::load_model(const std::string& path, std::vector<uint16_t>& indices) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<VertexData> vertices;

    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto& mesh = scene->mMeshes[i];

        // Process vertices
        for (size_t j = 0; j < mesh->mNumVertices; ++j) {
            VertexData vertex;
            vertex.vertices = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
            vertex.texCoord = mesh->mTextureCoords[0] ? glm::vec2{ mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y } : glm::vec2{ 0.0f, 0.0f };
            vertex.color = { 1.0f, 1.0f, 1.0f}; // Default white color
            vertices.push_back(vertex);
        }

        // Process indices
        for (size_t j = 0; j < mesh->mNumFaces; ++j) {
            const auto& face = mesh->mFaces[j];
            for (size_t k = 0; k < face.mNumIndices; ++k) {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    generateNormals(vertices, indices);

    return vertices;
}
