#include "renderer/context/fence.h"
#include "renderer/context/device.h"

estun::Fence::Fence(const bool signaled) 
{
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	VK_CHECK_RESULT(vkCreateFence(DeviceLocator::GetLogicalDevice(), &fenceInfo, nullptr, &fence), "Failed to create fence");
}

estun::Fence::Fence(Fence&& other) noexcept :
	fence(other.fence)
{
	other.fence = nullptr;
}

estun::Fence::~Fence()
{
	if (fence != nullptr)
	{
		vkDestroyFence(DeviceLocator::GetLogicalDevice(), fence, nullptr);
		fence = nullptr;
	}
}

void estun::Fence::Reset()
{
	VK_CHECK_RESULT(vkResetFences(DeviceLocator::GetLogicalDevice(), 1, &fence), "Failed to reset fence");
}

void estun::Fence::Wait(const uint64_t timeout) const
{
	VK_CHECK_RESULT(vkWaitForFences(DeviceLocator::GetLogicalDevice(), 1, &fence, VK_TRUE, timeout), "Failed to wait for fence");
}

const VkFence &estun::Fence::GetFence() const 
{ 
    return fence; 
}