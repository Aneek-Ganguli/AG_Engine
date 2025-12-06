#pragma once
#include <vector>

#include "VertexData.hpp"

struct ModelData {
    std::vector<AG_Engine::VertexData> vertexData{};
    std::vector<Uint32> indices{};
};

ModelData loadModel(const std::string& path);