#pragma once

#include "renderer/common.h"

namespace estun
{

class BaseImage;

class ImageView
{
public:
	ImageView(const ImageView &) = delete;
	ImageView(ImageView &&) = delete;

	ImageView &operator=(const ImageView &) = delete;
	ImageView &operator=(ImageView &&) = delete;

	explicit ImageView(VkImage *image, const uint32_t mipLevels);
	explicit ImageView(const VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	explicit ImageView(BaseImage *image, VkImageAspectFlags aspectFlags);

	~ImageView();

	VkImageView GetImageView() const;

private:
	VkImageView imageView;
};

} // namespace estun
