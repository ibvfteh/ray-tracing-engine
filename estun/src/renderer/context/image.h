#pragma once

#include "renderer/common.h"
#include "renderer/material/descriptable.h"

namespace estun
{

    class BaseImage;
    class ImageView;
    class DeviceMemory;
    class Sampler;

    class Image : public Descriptable
    {
    public:
        Image(const Image &) = delete;
        Image(Image &&) = delete;
        Image &operator=(const Image &) = delete;
        Image &operator=(Image &&) = delete;

        Image(const uint32_t width, const uint32_t height,
                    VkSampleCountFlagBits numSamples, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VkImageAspectFlags aspectFlags);
        ~Image();

        static std::shared_ptr<Image> CreateStorageImage(const uint32_t width, const uint32_t height);

        DescriptableInfo GetInfo() override;
        void ToLayout(const VkImageLayout &newLayout);
        void Barrier(
            VkCommandBuffer &commandBuffer,
            const VkImageLayout &newLayout,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkPipelineStageFlags sourceStage,
            VkPipelineStageFlags destinationStage);
        void CopyTo(
            VkCommandBuffer &commandBuffer,
            std::shared_ptr<Image> imageH);

        const BaseImage &GetImage() const  { return *image_; }
        const std::unique_ptr<ImageView> &GetImageView() const { return imageView_; }
        const Sampler &GetSampler() const { return *sampler_; }
        const VkImageLayout &GetLayout() const { return layout_; }

    protected:
        VkImageLayout layout_;
        std::unique_ptr<BaseImage> image_;
        std::unique_ptr<DeviceMemory> imageMemory_;
        std::unique_ptr<ImageView> imageView_;
        std::unique_ptr<Sampler> sampler_;
    };

} // namespace estun
