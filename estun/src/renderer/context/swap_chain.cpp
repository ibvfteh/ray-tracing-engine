#include "renderer/context/swap_chain.h"
#include "renderer/context/device.h"
#include "renderer/context/surface.h"
#include "renderer/context/device.h"
#include "renderer/context/image_view.h"

#undef max
#undef min
#include <algorithm>

estun::SwapChain::SwapChain(
    Surface *surface,
    const uint32_t &width,
    const uint32_t &height,
    bool vsync)
{
    QueueFamilyIndices indices = DeviceLocator::GetDevice().GetQueueFamilyIndices();
    SwapChainSupportDetails details = Device::QuerySwapChainSupport(DeviceLocator::GetPhysicalDevice(), surface);

    const auto surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
    const auto presentMode = ChooseSwapPresentMode(details.presentModes, vsync);
    const auto imageCount = ChooseImageCount(details.capabilities);
    extent = ChooseSwapExtent(details.capabilities, width, height);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->GetSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;

    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VK_CHECK_RESULT(vkCreateSwapchainKHR(DeviceLocator::GetLogicalDevice(), &createInfo, nullptr, &swapChain), "Failed to create swap chain!");

    minImageCount = imageCount;

    format = surfaceFormat.format;

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(DeviceLocator::GetLogicalDevice(), swapChain, &minImageCount, nullptr), "Failed to create swap chain images");
    images.resize(imageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(DeviceLocator::GetLogicalDevice(), swapChain, &minImageCount, images.data()), "Failed to get swap chain images");

    imageViews.reserve(images.size());

    for (const auto image : images)
    {
        imageViews.push_back(std::make_unique<ImageView>(image, format, VK_IMAGE_ASPECT_COLOR_BIT));
    }
}

estun::SwapChain::~SwapChain()
{
    imageViews.clear();

    if (swapChain != nullptr)
    {
        vkDestroySwapchainKHR(DeviceLocator::GetLogicalDevice(), swapChain, nullptr);
        swapChain = nullptr;
    }
}

VkSurfaceFormatKHR estun::SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats)
{
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto &format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    ES_CORE_ASSERT("Found no suitable surface format");
}

VkPresentModeKHR estun::SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &presentModes, const bool vsync)
{
    if (vsync)
    {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
    {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end())
    {
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_FIFO_RELAXED_KHR) != presentModes.end())
    {
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D estun::SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const uint32_t &width, const uint32_t &height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {width, height};

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

uint32_t estun::SwapChain::ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities)
{
    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    return imageCount;
}

uint32_t estun::SwapChain::GetMinImageCount() const
{
    return minImageCount;
}

const std::vector<VkImage> &estun::SwapChain::GetImages() const
{
    return images;
}

const std::vector<std::unique_ptr<estun::ImageView>> &estun::SwapChain::GetImageViews() const
{
    return imageViews;
}

const VkExtent2D &estun::SwapChain::GetExtent() const
{
    return extent;
}

VkFormat estun::SwapChain::GetFormat() const
{
    return format;
}

VkSwapchainKHR estun::SwapChain::GetSwapChain() const
{
    return swapChain;
}
