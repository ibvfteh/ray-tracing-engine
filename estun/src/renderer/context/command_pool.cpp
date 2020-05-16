#include "renderer/context/command_pool.h"
#include "renderer/context/device.h"

estun::CommandPool::CommandPool(CommandPoolType type)
{
    QueueFamilyIndices queueFamilyIndices = DeviceLocator::GetDevice().GetQueueFamilyIndices();
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    switch (type)
    {
    case Graphics:
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        break;
    case Compute:
        poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
        break;
    case Transfer:
        poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
        break;
    }
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK_RESULT(vkCreateCommandPool(DeviceLocator::GetLogicalDevice(), &poolInfo, nullptr, &commandPool), "Failed to create command pool");
}

estun::CommandPool::~CommandPool()
{
    vkDestroyCommandPool(DeviceLocator::GetLogicalDevice(), commandPool, nullptr);
}

void estun::CommandPool::Reset()
{
    vkResetCommandPool(DeviceLocator::GetLogicalDevice(), commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

VkCommandPool estun::CommandPool::GetCommandPool() const
{
    return commandPool;
}

estun::CommandPool *estun::CommandPoolLocator::currGraphicsCommandPool = nullptr;

estun::CommandPool *estun::CommandPoolLocator::currComputeCommandPool = nullptr;

estun::CommandPool *estun::CommandPoolLocator::currTransferCommandPool = nullptr;

estun::CommandPool &estun::CommandPoolLocator::GetGraphicsPool()
{
    if (currGraphicsCommandPool == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan graphics command pool");
    }
    return *currGraphicsCommandPool;
}

estun::CommandPool &estun::CommandPoolLocator::GetComputePool()
{
    if (currComputeCommandPool == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan graphics command pool");
    }
    return *currComputeCommandPool;
}

estun::CommandPool &estun::CommandPoolLocator::GetTransferPool()
{
    if (currTransferCommandPool == nullptr)
    {
        ES_CORE_ASSERT("Failed to request vulkan graphics command pool");
    }
    return *currTransferCommandPool;
}

void estun::CommandPoolLocator::Provide(
    estun::CommandPool *graphicsCommandPool,
    estun::CommandPool *computeCommandPool,
    estun::CommandPool *transferCommandPool)
{
    currGraphicsCommandPool = graphicsCommandPool;
    currComputeCommandPool = computeCommandPool;
    currTransferCommandPool = transferCommandPool;
}
