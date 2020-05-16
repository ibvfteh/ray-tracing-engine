#include "renderer/context/surface.h"
#include "renderer/context/instance.h"
#include "renderer/context/utils.h"

estun::Surface::Surface(estun::Instance *instance, GLFWwindow *window)
{
    VK_CHECK_RESULT(glfwCreateWindowSurface(*instance->GetVulkanInstance(), window, nullptr, &surface), "Failed to create window surface!");
}

void estun::Surface::Delete(Instance *instance)
{
    vkDestroySurfaceKHR(*instance->GetVulkanInstance(), surface, nullptr);
}

VkSurfaceKHR estun::Surface::GetSurface() const
{
    return surface;
}
