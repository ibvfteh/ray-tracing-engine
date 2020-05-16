#define STB_IMAGE_IMPLEMENTATION
#include "renderer/material/texture.h"
#include "core/core.h"
#include "renderer/context/base_image.h"
#include "renderer/context/image_view.h"
#include "renderer/material/sampler.h"
#include "renderer/device_memory.h"
#include "renderer/material/descriptable.h"
#include "renderer/context/command_pool.h"
#include "renderer/buffers/buffer.h"

estun::Texture::Texture(const std::string &filename, const estun::SamplerConfig &samplerConfig)
    : samplerConfig_(samplerConfig)
{

    // Load the texture in normal host memory.
    int width, height, channels;
    const auto pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels)
    {
        ES_CORE_ASSERT("failed to load texture image '" + filename + "'");
    }

    const VkDeviceSize imageSize = width * height * 4;

    auto stagingBuffer = std::make_unique<Buffer>(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    const auto data = stagingBufferMemory.Map(0, imageSize);
    std::memcpy(data, pixels, imageSize);
    stagingBufferMemory.Unmap();

    stbi_image_free(pixels);

    // Create the device side image, memory, view and sampler.
    image_.reset(new BaseImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height), VK_FORMAT_R8G8B8A8_UNORM));
    imageMemory_.reset(new DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
    imageView_.reset(new ImageView(image_.get(), VK_IMAGE_ASPECT_COLOR_BIT));
    sampler_.reset(new Sampler(samplerConfig_));

    // Transfer the data to device side.
    image_->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    image_->CopyFrom(*stagingBuffer);
    image_->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Delete the buffer before the memory
    stagingBuffer.reset();
}

estun::Texture::~Texture()
{
    sampler_.reset();
    imageView_.reset();
    image_.reset();
    imageMemory_.reset();
}

estun::Texture::Texture(Texture &&other) noexcept
    : sampler_(std::move(other.sampler_)),
      imageView_(std::move(other.imageView_)),
      image_(std::move(other.image_)),
      imageMemory_(std::move(other.imageMemory_))
{
}

estun::DescriptableInfo estun::Texture::GetInfo()
{
    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView_->GetImageView();
    imageInfo.sampler = sampler_->GetSampler();

    DescriptableInfo info;
    info.iI = imageInfo;

    return info;
}