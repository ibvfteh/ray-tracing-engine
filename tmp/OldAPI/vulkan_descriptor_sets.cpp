#include "renderer/material/DescriptorSets.h"
#include "renderer/context/Device.h"
//#include "renderer/context.h"

namespace estun
{
    VkDescriptorSetLayoutBinding VulkanDescriptorSets::UboBinding()
    {
        return Binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    }

    VkDescriptorSetLayoutBinding VulkanDescriptorSets::SamplerLayoutBinding()
    {
        return Binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    VkDescriptorSetLayoutBinding VulkanDescriptorSets::Binding(VkDescriptorType type, VkShaderStageFlags flags)
    {
        VkDescriptorSetLayoutBinding binding = {};
        //binding.binding = 1;
        binding.descriptorCount = 1;
        binding.descriptorType = type;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = flags;

        return binding;
    }

    VulkanDescriptorSets::VulkanDescriptorSets(std::vector<VkDescriptorSetLayoutBinding> bindings)
    {
        for (int i = 0; i < bindings.size(); i++)
        {
            bindings[i].binding = i;
        }
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*VulkanDeviceLocator::GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout), "Failed to create descriptor set layout");
    }

    VulkanDescriptorSets::~VulkanDescriptorSets()
    {
	    vkDestroyDescriptorSetLayout(*VulkanDeviceLocator::GetLogicalDevice(), descriptorSetLayout, nullptr);   
    }

    VkDescriptorSetLayout* VulkanDescriptorSets::GetDescriptorSetLayout()
    {
	    return &descriptorSetLayout;
    }

    const std::vector<VkDescriptorSet>* VulkanDescriptorSets::GetDescriptorSets() const
    {
    	return &descriptorSets;
    }		

	void VulkanDescriptorSets::CreateDescriptorSets(VkDescriptorPool* descriptorPool,
		                      const std::vector<VkBuffer>* uniformBuffers,
		                      VkImageView* textureImageView, VkSampler* textureSampler,
		                      const VkDeviceSize& bufferInfoSize)
    {
        uint32_t swapChainImagesSize = VulkanContextLocator::GetContext()->GetImageView()->GetSwapChainImageViewsVector()->size(); 
        std::vector<VkDescriptorSetLayout> layouts(swapChainImagesSize, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImagesSize);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapChainImagesSize);
        if (vkAllocateDescriptorSets(*VulkanDeviceLocator::GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to allocate descriptor sets");
        }

        for (size_t i = 0; i < swapChainImagesSize; i++) 
        {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = (*uniformBuffers)[i];
            bufferInfo.offset = 0;
            bufferInfo.range = bufferInfoSize;

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = *textureImageView;
            imageInfo.sampler = *textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(*VulkanDeviceLocator::GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanDescriptorSets::CreateComputeDescriptorSets(
        VkDescriptorPool* descriptorPool,
        VkBuffer* vertexBufferA,
        VkBuffer* vertexBufferB,
        VkBuffer* vertexBufferC,
        const VkDeviceSize& bufferInfoSize
        )
    {
        std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(1);
        if (vkAllocateDescriptorSets(*VulkanDeviceLocator::GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to allocate descriptor sets");
        }

        UpdateCompuiteDescriptorSets(
            vertexBufferA,
            vertexBufferB,
            vertexBufferC,
            bufferInfoSize);
    }

    void VulkanDescriptorSets::UpdateCompuiteDescriptorSets(
        VkBuffer* vertexBufferA,
        VkBuffer* vertexBufferB,
        VkBuffer* vertexBufferC,
        const VkDeviceSize& bufferInfoSize
        )
    {
      
        VkDescriptorBufferInfo bufferInfoA = {};
        bufferInfoA.buffer = *vertexBufferA;
        bufferInfoA.offset = 0;
        bufferInfoA.range = bufferInfoSize;

        VkDescriptorBufferInfo bufferInfoB = {};
        bufferInfoB.buffer = *vertexBufferB;
        bufferInfoB.offset = 0;
        bufferInfoB.range = bufferInfoSize;

        VkDescriptorBufferInfo bufferInfoC = {};
        bufferInfoC.buffer = *vertexBufferC;
        bufferInfoC.offset = 0;
        bufferInfoC.range = bufferInfoSize;

        std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[0];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoA;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[0];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfoB;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[0];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &bufferInfoC;

        vkUpdateDescriptorSets(*VulkanDeviceLocator::GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);  
    }
}
