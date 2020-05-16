#pragma once

#include "renderer/common.h"
#include "stb_image.h"
#include "renderer/material/descriptable.h"
#include "renderer/material/sampler.h"

namespace estun
{

class BaseImage;
class ImageView;
class Sampler;
class DeviceMemory;
class CommandPool;
class SamplerConfig;

class Texture : public Descriptable
{
public:
    Texture(const Texture &) = delete;
    Texture(Texture && other) noexcept;
    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    Texture(const std::string &filename, const SamplerConfig &samplerConfig = SamplerConfig());
    ~Texture();

    DescriptableInfo GetInfo() override;

    const ImageView &GetImageView() const { return *imageView_; }
    const Sampler &GetSampler() const { return *sampler_; }

private:
    SamplerConfig samplerConfig_;

    std::unique_ptr<BaseImage> image_;
    std::unique_ptr<DeviceMemory> imageMemory_;
    std::unique_ptr<ImageView> imageView_;
    std::unique_ptr<Sampler> sampler_;
};

} // namespace estun