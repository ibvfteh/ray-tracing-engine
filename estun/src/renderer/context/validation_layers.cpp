#include "renderer/context/validation_layers.h"
#include "renderer/context/instance.h"
#include "renderer/context/utils.h"
#include "core/core.h"
#include <iostream>

estun::ValidationLayers::ValidationLayers()
{
    if (enableValidationLayers && !CheckValidationLayers())
    {
        ES_CORE_ASSERT("Validation layer not supported");
    }
}

void estun::ValidationLayers::Delete(Instance *instance)
{
    DestroyDebugUtilsMessengerEXT(*instance->GetVulkanInstance(), debugMessenger, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL estun::ValidationLayers::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        ES_CORE_ERROR("Validation layer:");
        ES_CORE_ERROR(pCallbackData->pMessage);
    }
    else
    {
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            ES_CORE_WARN("Validation layer:");
            ES_CORE_WARN(pCallbackData->pMessage);
        }
        else
        {
            ES_CORE_INFO("Validation layer:");
            ES_CORE_INFO(pCallbackData->pMessage);
        }
    }
    return VK_FALSE;
}

bool estun::ValidationLayers::CheckValidationLayers()
{

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }
    return true;
}

void estun::ValidationLayers::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult estun::ValidationLayers::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        ES_CORE_ASSERT("Validation layer extension not present");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void estun::ValidationLayers::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void estun::ValidationLayers::SetupDebugMessenger(Instance *instance)
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(createInfo);
    VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(*instance->GetVulkanInstance(), &createInfo, nullptr, &debugMessenger), "Failed to set up debug messenger")
}

bool estun::ValidationLayers::IsEnabled()
{
    return enableValidationLayers;
}

std::vector<const char *> *estun::ValidationLayers::GetLayers()
{
    return &validationLayers;
}
