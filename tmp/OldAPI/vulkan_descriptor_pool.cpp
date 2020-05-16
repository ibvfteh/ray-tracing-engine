#include "renderer/material/vulkan_descriptor_pool.h"
#include "renderer/vulkan/vulkan_context.h"

namespace estun
{
    VkDescriptorPoolSize VulkanDescriptorPool::Descriptor(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = type;
        poolSize.descriptorCount = count;
        return poolSize;
    }

    VkDescriptorPoolSize VulkanDescriptorPool::UniformDescriptor()
    {
        uint32_t swapChainImagesSize = VulkanContextLocator::GetContext()->GetImageView()->GetSwapChainImageViewsVector()->size(); 
        return Descriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapChainImagesSize));
    }

    VkDescriptorPoolSize VulkanDescriptorPool::ImageDescriptor()
    {
        uint32_t swapChainImagesSize = VulkanContextLocator::GetContext()->GetImageView()->GetSwapChainImageViewsVector()->size(); 
        return Descriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapChainImagesSize));
    }

    VkDescriptorPoolSize VulkanDescriptorPool::StorageDescriptor()
    {
        return Descriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    }

    VulkanDescriptorPool::VulkanDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes)
    {
        uint32_t swapChainImagesSize = VulkanContextLocator::GetContext()->GetImageView()->GetSwapChainImageViewsVector()->size(); 
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(swapChainImagesSize);

        if (vkCreateDescriptorPool(*VulkanDeviceLocator::GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            ES_CORE_ASSERT("Failed to create descriptor pool");
        }
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        vkDestroyDescriptorPool(*VulkanDeviceLocator::GetLogicalDevice(), descriptorPool, nullptr);
    }

    VkDescriptorPool* VulkanDescriptorPool::GetDescriptorPool()
    {
        return &descriptorPool;
    }
}
