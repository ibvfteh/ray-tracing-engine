#pragma once

#include "renderer/common.h"

namespace estun
{

class Surface;
class Device;
class ImageView;

class SwapChain
{
public:
    SwapChain(const SwapChain &) = delete;
    SwapChain(SwapChain &&) = delete;

    SwapChain &operator=(const SwapChain &) = delete;
    SwapChain &operator=(SwapChain &&) = delete;

    SwapChain(
        Surface *surface,
        const uint32_t &width,
        const uint32_t &height,
        bool vsync);
    ~SwapChain();

    uint32_t GetMinImageCount() const;
    const std::vector<VkImage> &GetImages() const;
    const std::vector<std::unique_ptr<ImageView>> &GetImageViews() const;
    const VkExtent2D &GetExtent() const;
    VkFormat GetFormat() const;

    VkSwapchainKHR GetSwapChain() const;

private:
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &presentModes, bool vsync);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const uint32_t &width, const uint32_t &height);
    static uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);

    VkSwapchainKHR swapChain;

    uint32_t minImageCount;
    VkFormat format;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<std::unique_ptr<ImageView>> imageViews;
};

} // namespace estun
