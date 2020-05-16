#pragma once

#include "renderer/common.h"

namespace estun
{
    struct Shader
    {
        std::string name;
        VkShaderStageFlagBits bits;
    };
}