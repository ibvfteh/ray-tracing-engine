#include "renderer/context/resources.h"
#include "renderer/context/device.h"
#include "renderer/context/base_image.h"
#include "renderer/context/image_view.h"
#include "renderer/context/swap_chain.h"
#include "renderer/device_memory.h"
#include "renderer/context/image.h"
#include "renderer/context.h"

estun::DepthResources::DepthResources(const VkExtent2D &extent, VkSampleCountFlagBits msaaSamples)
    : Image(extent.width,
                  extent.height,
                  msaaSamples,
                  FindDepthFormat(),
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_DEPTH_BIT)
{
    layout_ =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

estun::DepthResources::~DepthResources()
{
}

VkFormat estun::DepthResources::FindDepthFormat()
{
    return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat estun::DepthResources::FindSupportedFormat(
    const std::vector<VkFormat> &candidates,
    VkImageTiling tiling,
    const VkFormatFeatureFlags &features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(DeviceLocator::GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    ES_CORE_ASSERT("Failed to find supported format");
}

bool estun::DepthResources::HasStencilComponent(const VkFormat &format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

estun::ColorResources::ColorResources(const VkExtent2D &extent, VkSampleCountFlagBits msaaSamples)
    : Image(extent.width,
                  extent.height,
                  msaaSamples,
                  ContextLocator::GetSwapChain()->GetFormat(),
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_COLOR_BIT)
{
    layout_ =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

estun::ColorResources::~ColorResources()
{
}

