#include "renderer/vulkan/vulkan_material.h"
#include "renderer/material/vulkan_material_pool.h"
#include "renderer/material/vulkan_pipeline.h"


namespace estun
{
    VulkanMaterial::VulkanMaterial(bool compute)
    {
        hasCompute = compute;
        descriptorSets = new VulkanDescriptorSets({VulkanDescriptorSets::UboBinding(), VulkanDescriptorSets::SamplerLayoutBinding()}); 
        if (hasCompute)
        {
            computeDescriptorSets = new VulkanDescriptorSets({
                VulkanDescriptorSets::Binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT),
                VulkanDescriptorSets::Binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT),
                VulkanDescriptorSets::Binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                }); 
        }
    }

    VulkanMaterial::~VulkanMaterial()
    {
        DeletePipeline();
        if (hasCompute)
        {
            delete computeDescriptorSets;
            delete computeDescriptorPool;
        }
        delete descriptorSets;
        delete descriptorPool;
    }

    void VulkanMaterial::CreatePipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    {
        pipeline = VulkanMaterialPoolLocator::GetPipeline(vertexShaderPath, fragmentShaderPath, descriptorSets);
    }

    void VulkanMaterial::CreateComputePipeline(const std::string& computeShaderPath)
    {
        if (!hasCompute)
        {
            ES_CORE_ASSERT("Compute pipeline creation in non compute material")
        }
        computePipeline = std::make_shared<VulkanComputePipeline>(computeShaderPath, computeDescriptorSets);
    }

    void VulkanMaterial::CreateTexture(const std::string& texturePath)
    {
        textureImage = VulkanMaterialPoolLocator::GetTextureImage(texturePath);
        textureImageView = VulkanMaterialPoolLocator::GetTextureImageView(texturePath);
        textureSampler = VulkanMaterialPoolLocator::GetTextureSampler(texturePath);
    }

    void VulkanMaterial::CreateDescriptorSets(VulkanUniformBuffer* uniformBuffer)
    {
    	descriptorPool = new VulkanDescriptorPool({VulkanDescriptorPool::UniformDescriptor(), VulkanDescriptorPool::ImageDescriptor()});
	    descriptorSets->CreateDescriptorSets(
	            descriptorPool->GetDescriptorPool(), 
	            uniformBuffer->GetUniformBuffers(),
	            textureImageView->GetTextureImageView(),
	            textureSampler->GetTextureSampler());
    }
    
    void VulkanMaterial::CreateComputeDescriptorSets(
        //VulkanUniformBuffer* computeUniformBuffer
        VulkanVertexBuffer* vboA,
        VulkanVertexBuffer* vboB,
        VulkanVertexBuffer* vboC
        )
    {
    	computeDescriptorPool = new VulkanDescriptorPool({
            VulkanDescriptorPool::StorageDescriptor(),
            VulkanDescriptorPool::StorageDescriptor(),
            VulkanDescriptorPool::StorageDescriptor()
        });
        if (hasCompute)
        {
	        computeDescriptorSets->CreateComputeDescriptorSets(
	            computeDescriptorPool->GetDescriptorPool(),
                vboA->GetBuffer(),
                vboB->GetBuffer(),
                vboC->GetBuffer(),
                vboA->GetSize()
	            //computeUniformBuffer->GetUniformBuffers()
                //computeUniformBuffer->GetSize()
                );
        }
        else
        {
            ES_CORE_ASSERT("Compute descriptorsets creation in non compute material");
        }
        
    }

    void VulkanMaterial::Update(
        VulkanVertexBuffer* vboA,
        VulkanVertexBuffer* vboB,
        VulkanVertexBuffer* vboC
        )
    {
        if (hasCompute)
        {
	        computeDescriptorSets->UpdateCompuiteDescriptorSets(
                vboA->GetBuffer(),
                vboB->GetBuffer(),
                vboC->GetBuffer(),
                vboA->GetSize()
                );
        }
    }

    void VulkanMaterial::DeletePipeline()
    {
        pipeline = nullptr;
    }

    std::shared_ptr<VulkanGraphicsPipeline> VulkanMaterial::GetPipeline() const
    {
        return pipeline;
    }

    std::shared_ptr<VulkanComputePipeline> VulkanMaterial::GetComputePipeline()
    {
        return computePipeline;
    }

    VulkanDescriptorSets* VulkanMaterial::GetDescriptorSets() 
    {
        return descriptorSets;
    }

    VulkanDescriptorSets* VulkanMaterial::GetComputeDescriptorSets() 
    {
        if (!hasCompute)
        {
            ES_CORE_ASSERT("Compute get descriptor set in non compute material")
        }
        return computeDescriptorSets;
    }
}
