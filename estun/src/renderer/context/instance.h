#pragma once

#include "core/core.h"
#include "renderer/common.h"

namespace estun
{

class ValidationLayers;

class Version
{
public:
    int major, minor, patch;

    Version(int ma = 0, int mi = 0, int pa = 0) : major(ma), minor(mi), patch(pa) {}
};

class Instance
{
private:
    VkInstance vk_instance;

    ValidationLayers *valLayers = nullptr;

    const std::vector<const char *> instanceExtensions = {
        "VK_KHR_get_physical_device_properties2"};

public:
    Instance(const Instance &) = delete;
    Instance(Instance &&) = delete;

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    Instance(
        const char *app_name,
        const Version app_version,
        const char *engine_name = "estun",
        const Version engine_version = Version(1, 0, 0));
    ~Instance();
    VkInstance *GetVulkanInstance();
    ValidationLayers *GetValidationLayers();

private:
    void CreateInstance(const char *app_name,
                        const Version app_version,
                        const char *engine_name,
                        const Version engine_version);
};

} // namespace estun
