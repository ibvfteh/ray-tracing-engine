#include "renderer/context/instance.h"
#include "renderer/context/utils.h"
#include "renderer/context/validation_layers.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>

estun::Instance::Instance(const char *app_name,
                          const Version app_version,
                          const char *engine_name,
                          const Version engine_version)
{
    if (ValidationLayers::IsEnabled())
    {
        valLayers = new ValidationLayers();
    }
    CreateInstance(app_name, app_version, engine_name, engine_version);
    if (valLayers)
    {
        valLayers->SetupDebugMessenger(this);
    }
}

estun::Instance::~Instance()
{
    if (valLayers)
    {
        valLayers->Delete(this);
        delete valLayers;
    }

    vkDestroyInstance(vk_instance, nullptr);
}

std::vector<const char *> GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (estun::ValidationLayers::IsEnabled())
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void estun::Instance::CreateInstance(const char *app_name,
                                     const Version app_version,
                                     const char *engine_name,
                                     const Version engine_version)

{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = app_name;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(app_version.major, app_version.minor, app_version.patch);
    applicationInfo.pEngineName = engine_name;
    applicationInfo.engineVersion = VK_MAKE_VERSION(engine_version.major, engine_version.minor, engine_version.patch);
    applicationInfo.apiVersion = VK_API_VERSION_1_2; //TODO VK_API_VERSION_1_2

    auto extensions = GetRequiredExtensions();
    for (auto extention : instanceExtensions)
    {
        extensions.push_back(extention);
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    std::vector<const char *> enabledLayers;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (valLayers)
    {
        enabledLayers = *valLayers->GetLayers();

        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();

        valLayers->populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &vk_instance), "Failed to create vulkan instance");
}

VkInstance *estun::Instance::GetVulkanInstance()
{
    return &vk_instance;
}

estun::ValidationLayers *estun::Instance::GetValidationLayers()
{
    return valLayers;
}
