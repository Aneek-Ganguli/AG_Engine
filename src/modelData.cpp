#include <iostream>
#include <ModelData.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

ModelData loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<AG_Engine::VertexData> vertices;
    std::vector<Uint32> indices;

    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto& mesh = scene->mMeshes[i];

        // Process vertices
        for (size_t j = 0; j < mesh->mNumVertices; ++j) {
            AG_Engine::VertexData vertex;
            vertex.position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
            vertex.texCoord = mesh->mTextureCoords[0] ? vec2{ mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y } : vec2{ 0.0f, 0.0f };
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

    return {.vertexData = vertices,.indices = indices};
}
