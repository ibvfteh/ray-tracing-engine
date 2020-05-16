#pragma once

#include "core/core.h"
#include "renderer/common.h"
#include "renderer/context/utils.h"

namespace estun
{

class Instance;
class Surface;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
private:
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSampleCountFlagBits msaaSamples;

    QueueFamilyIndices currIndices;

    const std::vector<const char *> deviceExtensions = {
        "VK_KHR_swapchain",
        "VK_KHR_maintenance3",
        "VK_KHR_get_memory_requirements2",
        "VK_EXT_descriptor_indexing",
        "VK_KHR_buffer_device_address",
        "VK_KHR_deferred_host_operations",
        "VK_KHR_pipeline_library",
        "VK_KHR_ray_tracing",
        //"VK_NV_ray_tracing",
    };

    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

public:
    Device(const Device &) = delete;
    Device(Device &&) = delete;

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    Device(Instance *instance, Surface *surface);
    ~Device();

    VkPhysicalDevice &GetPhysicalDevice();
    VkDevice &GetLogicalDevice();

    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, Surface *surface);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    QueueFamilyIndices GetQueueFamilyIndices() const;

    VkSampleCountFlagBits GetMaxUsableSampleCount();
    VkSampleCountFlagBits GetMsaaSamples() const;

    VkQueue GetGraphicsQueue();
    VkQueue GetComputeQueue();
    VkQueue GetPresentQueue();
    VkQueue GetTransferQueue();

    void WaitIdle() const;

private:
    void PickPhysicalDevice(Instance *instance, Surface *surface);
    void CreateLogicalDevice(Instance *instance, Surface *surface);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device, Surface *surfcae);

    uint32_t GetQueueFamilyIndex(std::vector<VkQueueFamilyProperties> queueFamilyProperties, VkQueueFlagBits queueFlags);
};

class DeviceLocator
{
public:
    static Device &GetDevice();
    static VkDevice &GetLogicalDevice();
    static VkPhysicalDevice &GetPhysicalDevice();

    static void Provide(Device *device) { currDevice = device; };

private:
    static Device *currDevice;
};

} // namespace estun