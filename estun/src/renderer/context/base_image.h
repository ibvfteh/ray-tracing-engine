#pragma once

#include "renderer/common.h"

namespace estun
{

class Buffer;
class DeviceMemory;

class BaseImage
{
private:
	VkImage image;
	uint32_t width_;
	uint32_t height_;
	const VkFormat format_;
	VkImageLayout layout_;
	uint32_t mipLevels_;

public:
	BaseImage(const BaseImage &) = delete;
	BaseImage(BaseImage &&other) noexcept;

	BaseImage &operator=(const BaseImage &) = delete;
	BaseImage &operator=(BaseImage &&) = delete;

	BaseImage(const uint32_t width, const uint32_t height, VkFormat format);
	BaseImage(const uint32_t width, const uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
	BaseImage(const uint32_t width, const uint32_t height, const uint32_t mipLevels,
		  VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		  VkImageUsageFlags usage);

	~BaseImage();

	DeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
	VkMemoryRequirements GetMemoryRequirements() const;

	void TransitionImageLayout(const VkImageLayout &newLayout);
	void CopyFrom(const Buffer &buffer);
	void GenerateMipmaps();

	VkImage GetImage() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	VkFormat GetFormat();
	VkImageLayout GetLayout();
	uint32_t GetMipLevels();

	void SetLayout(VkImageLayout layout);
};

} // namespace estun
