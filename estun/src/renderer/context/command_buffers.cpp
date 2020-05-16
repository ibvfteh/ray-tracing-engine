#include "renderer/context/command_buffers.h"
#include "renderer/context/command_pool.h"
#include "renderer/context/device.h"

estun::CommandBuffers::CommandBuffers(CommandPool& commandPool, const uint32_t size) :
	commandPool_(commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool_.GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = size;

	commandBuffers.resize(size);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(DeviceLocator::GetLogicalDevice(), &allocInfo, commandBuffers.data()), "Failed to allocate command buffers");
}

estun::CommandBuffers::~CommandBuffers()
{
	if (!commandBuffers.empty())
	{
		vkFreeCommandBuffers(DeviceLocator::GetLogicalDevice(), commandPool_.GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}
}

void estun::CommandBuffers::Begin(const size_t i)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "Failed to begin recording command buffer");
}

void estun::CommandBuffers::End(const size_t i)
{
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer");
}