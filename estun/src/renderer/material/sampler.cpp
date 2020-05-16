#include "renderer/material/sampler.h"
#include "renderer/context/device.h"

estun::Sampler::Sampler(const estun::SamplerConfig &config)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = config.MagFilter;
    samplerInfo.minFilter = config.MinFilter;
    samplerInfo.addressModeU = config.AddressModeU;
    samplerInfo.addressModeV = config.AddressModeV;
    samplerInfo.addressModeW = config.AddressModeW;
    samplerInfo.anisotropyEnable = config.AnisotropyEnable;
    samplerInfo.maxAnisotropy = config.MaxAnisotropy;
    samplerInfo.borderColor = config.BorderColor;
    samplerInfo.unnormalizedCoordinates = config.UnnormalizedCoordinates;
    samplerInfo.compareEnable = config.CompareEnable;
    samplerInfo.compareOp = config.CompareOp;
    samplerInfo.mipmapMode = config.MipmapMode;
    samplerInfo.mipLodBias = config.MipLodBias;
    samplerInfo.minLod = config.MinLod;
    samplerInfo.maxLod = config.MaxLod;

    VK_CHECK_RESULT(vkCreateSampler(DeviceLocator::GetLogicalDevice(), &samplerInfo, nullptr, &sampler), "Failed to create sampler");
}

estun::Sampler::~Sampler()
{
    if (sampler != nullptr)
    {
        vkDestroySampler(DeviceLocator::GetLogicalDevice(), sampler, nullptr);
        sampler = nullptr;
    }
}

VkSampler estun::Sampler::GetSampler() const
{
    return sampler;
}