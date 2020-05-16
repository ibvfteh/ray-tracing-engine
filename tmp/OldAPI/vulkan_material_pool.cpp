#include "renderer/material/vulkan_material_pool.h"

#include <iostream>
#include <sstream>

namespace estun
{
    VulkanMaterialPool* VulkanMaterialPoolLocator::currPool = nullptr;

    VulkanMaterialPool::~VulkanMaterialPool()
    {
        CleanUp();
    }

	void VulkanMaterialPool::CleanUp()
    {
    	std::vector<std::string> textureToErase;
    	for (const auto& texture : textureImagePool)
	    {
		    if (texture.second.use_count() == 1)
		    {
			    textureToErase.push_back(texture.first);
		    }
	    }

    	for (const auto& textureKey : textureToErase)
    	{
		    textureImagePool.erase(textureKey);
		    textureImageViewPool.erase(textureKey);
		    textureSamplerPool.erase(textureKey);
	    }

	    std::vector<std::string> pipelineToErase;
	    for (const auto& pipeline : pipelinePool)
	    {
		    if (pipeline.second.use_count() == 1)
		    {
			    pipelineToErase.push_back(pipeline.first);
	    	}
	    }

	    for (const auto& pipelineKey : pipelineToErase)
	    {
		    pipelinePool.erase(pipelineKey);
    	}
    }
		
	void VulkanMaterialPool::RebuildPipelines() 
    {

        for (const auto& pipeline : pipelinePool)
        {
            std::istringstream iss(pipeline.first);
			std::string vertexShaderPath;
			std::string fragmentShaderPath;
            iss >> vertexShaderPath >> fragmentShaderPath;
            pipeline.second->RebuildPipeline(vertexShaderPath, fragmentShaderPath);
        }

    }

    std::shared_ptr<VulkanGraphicsPipeline> VulkanMaterialPool::GetPipeline(
				const std::string& vertexShaderPath,
				const std::string& fragmentShaderPath,
				VulkanDescriptorSets* descriptorSets)
    {
        const std::string keyString = vertexShaderPath + " " + fragmentShaderPath;
        if (pipelinePool.find(keyString) == pipelinePool.end())
	    {
	    	pipelinePool[keyString] = std::make_shared<VulkanGraphicsPipeline>(
			    vertexShaderPath,
		    	fragmentShaderPath,
			    descriptorSets);
		    ES_CORE_INFO("Pipeline loaded and created");
	    }
	    return pipelinePool[keyString];
    }

	std::shared_ptr<VulkanTextureImage> VulkanMaterialPool::GetTextureImage(const std::string& texturePath)
    {
       if (textureImagePool.find(texturePath) == textureImagePool.end())
       {
           textureImagePool[texturePath] = std::make_shared<VulkanTextureImage>(texturePath);
           ES_CORE_INFO("Texture image loaded and created");
       }
       return textureImagePool[texturePath];
    }

	std::shared_ptr<VulkanTextureSampler> VulkanMaterialPool::GetTextureSampler(const std::string& texturePath)
    {
       if (textureSamplerPool.find(texturePath) == textureSamplerPool.end())
       {
           const std::shared_ptr<VulkanTextureImage> textureImage = GetTextureImage(texturePath);
           textureSamplerPool[texturePath] = std::make_shared<VulkanTextureSampler>(textureImage->GetMipLevels());
           ES_CORE_INFO("Texture image sampler loaded and created");
       }
       return textureSamplerPool[texturePath];
    }

	std::shared_ptr<VulkanTextureImageView> VulkanMaterialPool::GetTextureImageView(const std::string& texturePath)
    {
       if (textureImageViewPool.find(texturePath) == textureImageViewPool.end())
       {
           const std::shared_ptr<VulkanTextureImage> textureImage = GetTextureImage(texturePath);
           textureImageViewPool[texturePath] = std::make_shared<VulkanTextureImageView>(
               textureImage->GetTextureImage(),
               textureImage->GetMipLevels()
               );
           ES_CORE_INFO("Texture image view loaded and created");
       }
       return textureImageViewPool[texturePath];
    }
}
