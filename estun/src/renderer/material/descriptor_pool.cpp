#include "renderer/material/descriptor_pool.h"
#include "renderer/context/device.h"
#include "renderer/material/descriptor_binding.h"

estun::DescriptorPool::DescriptorPool(const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets)
{
    std::vector<VkDescriptorPoolSize> poolSizes;

    for (const auto &binding : descriptorBindings)
    {
        poolSizes.push_back(VkDescriptorPoolSize{binding.type_, static_cast<uint32_t>(binding.descriptorCount_ * maxSets)});
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxSets);

    VK_CHECK_RESULT(vkCreateDescriptorPool(DeviceLocator::GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool");
}

estun::DescriptorPool::~DescriptorPool()
{
    if (descriptorPool != nullptr)
    {
        vkDestroyDescriptorPool(DeviceLocator::GetLogicalDevice(), descriptorPool, nullptr);
        descriptorPool = nullptr;
    }
}

VkDescriptorPool estun::DescriptorPool::GetDescriptorPool() const
{
    return descriptorPool;
}