
#pragma once

#include "renderer/model.h"
#include <vector>

class CornellBox
{
public:
    static estun::Model CreateCornellBox(const float scale);

    static void Create(
        float scale,
        std::vector<estun::Vertex> &vertices,
        std::vector<uint32_t> &indices,
        std::vector<estun::Material> &materials);
};
