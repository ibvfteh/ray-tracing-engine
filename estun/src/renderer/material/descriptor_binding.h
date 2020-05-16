#pragma once

#include "renderer/common.h"
#include "renderer/material/descriptable.h"
#include "renderer/buffers/uniform_buffer.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/material/texture.h"
#include "renderer/context/image.h"
#include "renderer/ray_tracing/top_level_acceleration_structure.h"
//#include "renderer/ray_tracing/TopLevelAccelerationStructure.h"

namespace estun
{

class Descriptable;
template <class T>
class UniformBuffer;
template <class T>
class StorageBuffer;
class Texture;
class Image;
class TopLevelAccelerationStructure;

struct DescriptorBinding
{
    uint32_t binding_; // Slot to which the descriptor will be bound, corresponding to the layout index in the shader.
    std::vector<Descriptable*> descriptable_;
    uint32_t descriptorCount_; // Number of descriptors to bind
    VkDescriptorType type_;    // Type of the bound descriptor(s)
    VkShaderStageFlags stage_; // Shader stage at which the bound resources will be available

    DescriptorBinding(uint32_t binding, std::vector<Descriptable*> descriptable, uint32_t descriptorCount, VkDescriptorType type, VkShaderStageFlags stage)
        : binding_(binding),
          descriptable_(descriptable),
          descriptorCount_(descriptorCount),
          type_(type),
          stage_(stage)
    {
    }

	template <class T>
    static DescriptorBinding Uniform(uint32_t binding, std::vector<UniformBuffer<T>> &uniformBuffers, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {};
        for (int i = 0; i < uniformBuffers.size(); i++)
        {
            descriptables.push_back(&uniformBuffers[i]);
        }
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage);
    }

    template <class T>
    static DescriptorBinding Storage(uint32_t binding, std::shared_ptr<StorageBuffer<T>> storageBuffer, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {storageBuffer.get()};
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stage);
    }

    static DescriptorBinding Texture(uint32_t binding, Texture &texture, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {&texture};
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage);
    }

    static DescriptorBinding Textures(uint32_t binding, std::vector<class Texture> &textures, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {};
        for (int i = 0; i < textures.size(); i++)
        {
            descriptables.push_back(&textures[i]);
        }
        return DescriptorBinding(binding, descriptables, static_cast<uint32_t>(textures.size()), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage);
    }

    static DescriptorBinding AccelerationStructure(uint32_t binding, std::shared_ptr<TLAS> tlas, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {tlas.get()};
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, stage);
    }

    static DescriptorBinding ImageSampler(uint32_t binding, std::shared_ptr<Image> Image, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {Image.get()};
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage);
    }

    static DescriptorBinding StorageImage(uint32_t binding, std::shared_ptr<Image> Image, VkShaderStageFlags stage)
    {
        std::vector<Descriptable*> descriptables = {Image.get()};
        return DescriptorBinding(binding, descriptables, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stage);
    }
};

} // namespace estun
