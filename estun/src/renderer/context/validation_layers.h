#pragma once

#include <vector>
#include "renderer/common.h"

namespace estun
{

class Instance;

class ValidationLayers
{
private:
    VkDebugUtilsMessengerEXT debugMessenger;

#ifdef NDEBUG
    static const bool enableValidationLayers = false;
#else
    static const bool enableValidationLayers = true;
#endif

    std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator);

public:
    ValidationLayers(const ValidationLayers &) = delete;
    ValidationLayers(ValidationLayers &&) = delete;

    ValidationLayers &operator=(const ValidationLayers &) = delete;
    ValidationLayers &operator=(ValidationLayers &&) = delete;

    ValidationLayers();
    ~ValidationLayers() = default;

    void Delete(Instance *instance);

    void SetupDebugMessenger(Instance *instance);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    bool CheckValidationLayers();
    static bool IsEnabled();
    std::vector<const char *> *GetLayers();
};

} // namespace estun
