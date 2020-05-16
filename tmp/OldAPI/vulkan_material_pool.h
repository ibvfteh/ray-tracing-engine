#pragma once

#include <vulkan/vulkan.h>

#include "core/core.h"
#include "renderer/material/vulkan_texture_image.h"
#include "renderer/material/vulkan_texture_image_view.h"
#include "renderer/material/vulkan_texture_sampler.h"
#include "renderer/material/vulkan_descriptor_sets.h"
#include "renderer/material/vulkan_pipeline.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace estun
{
    class VulkanMaterialPool
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<VulkanTextureImage>>     textureImagePool;
        std::unordered_map<std::string, std::shared_ptr<VulkanTextureImageView>> textureImageViewPool;
        std::unordered_map<std::string, std::shared_ptr<VulkanTextureSampler>>   textureSamplerPool;

        std::unordered_map<std::string, std::shared_ptr<VulkanGraphicsPipeline>> pipelinePool;
    public:
        VulkanMaterialPool() = default;
		~VulkanMaterialPool();

        std::shared_ptr<VulkanGraphicsPipeline> GetPipeline(
				const std::string& vertexShaderPath,
				const std::string& fragmentShaderPath,
				VulkanDescriptorSets* descriptorSets);

		std::shared_ptr<VulkanTextureImage>     GetTextureImage(const std::string& texturePath);
		std::shared_ptr<VulkanTextureSampler>   GetTextureSampler(const std::string& texturePath);
		std::shared_ptr<VulkanTextureImageView> GetTextureImageView(const std::string& texturePath);

		void CleanUp();
		void RebuildPipelines();
    };

    class VulkanMaterialPoolLocator
    {
    private:
        static VulkanMaterialPool* currPool;
    public:
        static void Provide(VulkanMaterialPool* pool) { currPool = pool; };

        static std::shared_ptr<VulkanGraphicsPipeline> GetPipeline(
			const std::string& vertexShaderPath,
			const std::string& fragmentShaderPath,
			VulkanDescriptorSets* descriptorSets)
        { return currPool->GetPipeline(vertexShaderPath, fragmentShaderPath, descriptorSets); };

		static std::shared_ptr<VulkanTextureImage>     GetTextureImage(const std::string& texturePath) { return currPool->GetTextureImage(texturePath); };
		static std::shared_ptr<VulkanTextureSampler>   GetTextureSampler(const std::string& texturePath) { return currPool->GetTextureSampler(texturePath); };
		static std::shared_ptr<VulkanTextureImageView> GetTextureImageView(const std::string& texturePath) { return currPool->GetTextureImageView(texturePath); };
		static void RebuildPipelines() { return currPool->RebuildPipelines(); };
    };
}
