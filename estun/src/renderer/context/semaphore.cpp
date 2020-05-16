#include "renderer/context/semaphore.h"
#include "renderer/context/device.h"

estun::Semaphore::Semaphore() 
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK_RESULT(vkCreateSemaphore(DeviceLocator::GetLogicalDevice(), &semaphoreInfo, nullptr, &semaphore), "Failed to create semaphores");
}

estun::Semaphore::Semaphore(Semaphore&& other) noexcept :
	semaphore(other.semaphore)
{
	other.semaphore = nullptr;
}

estun::Semaphore::~Semaphore()
{
	if (semaphore != nullptr)
	{
		vkDestroySemaphore(DeviceLocator::GetLogicalDevice(), semaphore, nullptr);
		semaphore = nullptr;
	}
}

VkSemaphore estun::Semaphore::GetSemaphore() const
{
    return semaphore;
}

/*
namespace estun
{
    VulkanSemaphoresManager::VulkanSemaphoresManager(const uint32_t imageCount_t, const uint32_t swapChainImagesSize)
        : imageCount(imageCount_t)
    {
        imageAvailableSemaphores.resize(imageCount);
        renderFinishedSemaphores.resize(imageCount);
        inFlightFences.resize(imageCount);
        imagesInFlight.resize(swapChainImagesSize, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < imageCount; i++) 
        {
            if (vkCreateSemaphore(*VulkanDeviceLocator::GetLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(*VulkanDeviceLocator::GetLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(*VulkanDeviceLocator::GetLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) 
            {
                ES_CORE_ASSERT("Failed to create synchronization objects for a frame");
            }
        }
        if (vkCreateSemaphore(*VulkanDeviceLocator::GetLogicalDevice(), &semaphoreInfo, nullptr, &computeAvailableSemaphores) != VK_SUCCESS ||
            vkCreateSemaphore(*VulkanDeviceLocator::GetLogicalDevice(), &semaphoreInfo, nullptr, &computeFinishedSemaphores) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to create synchronization objects for a frame");
        }
    }

    VulkanSemaphoresManager::~VulkanSemaphoresManager()
    {
        for (size_t i = 0; i < imageCount; i++) 
        {
            vkDestroySemaphore(*VulkanDeviceLocator::GetLogicalDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(*VulkanDeviceLocator::GetLogicalDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(*VulkanDeviceLocator::GetLogicalDevice(), inFlightFences[i], nullptr);
        }
        vkDestroySemaphore(*VulkanDeviceLocator::GetLogicalDevice(), computeAvailableSemaphores, nullptr);
        vkDestroySemaphore(*VulkanDeviceLocator::GetLogicalDevice(), computeFinishedSemaphores, nullptr);
    }

    const std::vector<VkSemaphore>* VulkanSemaphoresManager::GetImageAvailableSemaphores() const
    {
        return &imageAvailableSemaphores;
    }

    const std::vector<VkSemaphore>* VulkanSemaphoresManager::GetRenderFinishedSemaphores() const
    {
        return &renderFinishedSemaphores;
    }
        
    VkSemaphore* VulkanSemaphoresManager::GetComputeAvailableSemaphores() 
    {
        return &computeAvailableSemaphores;
    }
    
    VkSemaphore* VulkanSemaphoresManager::GetComputeFinishedSemaphores() 
    {
        return &computeFinishedSemaphores;
    }

    const std::vector<VkFence>* VulkanSemaphoresManager::GetInFlightFences() const
    {
        return &inFlightFences;
    }
    
    std::vector<VkFence>* VulkanSemaphoresManager::GetImagesInFlight() 
    {
        return &imagesInFlight;
    }
}
*/