#pragma once

#include "core/core.h"
#include "renderer/common.h"

namespace estun
{

#define VK_CHECK_RESULT(f, n)                                                                                                                                                                       \
    {                                                                                                                                                                                               \
        VkResult res = (f);                                                                                                                                                                         \
        if (res != VK_SUCCESS)                                                                                                                                                                      \
        {                                                                                                                                                                                           \
            ES_CORE_ASSERT(std::string(n) + std::string(":\n\tVkResult is \"") + ErrorString(res) + std::string("\" in ") + __FILE__ + std::string(" at line ") + std::to_string(__LINE__) + std::string("\n")); \
        }                                                                                                                                                                                           \
    }

    std::string ErrorString(VkResult errorCode);
} // namespace estun
