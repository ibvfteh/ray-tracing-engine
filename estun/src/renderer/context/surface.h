#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "renderer/common.h"

#include "core/core.h"
#include <GLFW/glfw3.h>

namespace estun
{

class Instance;

class Surface
{
private:
    VkSurfaceKHR surface;

public:
    Surface(const Surface &) = delete;
    Surface(Surface &&) = delete;

    Surface &operator=(const Surface &) = delete;
    Surface &operator=(Surface &&) = delete;

    Surface(Instance *instance, GLFWwindow *window);
    ~Surface() = default;

    void Delete(Instance *instance);

    VkSurfaceKHR GetSurface() const;
};

} // namespace estun
