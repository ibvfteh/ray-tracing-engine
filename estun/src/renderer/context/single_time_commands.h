#pragma once

#include "renderer/common.h"
#include "renderer/context/command_buffers.h"
#include "renderer/context/command_pool.h"
#include "renderer/context/device.h"
#include "renderer/context/utils.h"
#include "renderer/context/fence.h"

namespace estun
{

class SingleTimeCommands
{
public:
    static void SubmitGraphics(CommandPool &commandPool, const std::function<void(VkCommandBuffer)> &action)
    {
        CommandBuffers commandBuffers(commandPool, 1);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

        action(commandBuffers[0]);

        vkEndCommandBuffer(commandBuffers[0]);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        const auto queue = DeviceLocator::GetDevice().GetGraphicsQueue();

        vkQueueSubmit(queue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(queue);
    }

    static void SubmitCompute(const std::function<void(VkCommandBuffer)> &action, std::string name)
    {
        CommandBuffers commandBuffers(CommandPoolLocator::GetComputePool(), 1);
        Fence fence(false);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

        action(commandBuffers[0]);

        vkEndCommandBuffer(commandBuffers[0]);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        const auto queue = DeviceLocator::GetDevice().GetComputeQueue();
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence.GetFence()), std::string("Failed to submit in queue with command: ") + name);
        fence.Wait(UINT64_MAX);
        vkQueueWaitIdle(queue);
    }

    static void SubmitTransfer(CommandPool &commandPool, const std::function<void(VkCommandBuffer)> &action)
    {
        CommandBuffers commandBuffers(commandPool, 1);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

        action(commandBuffers[0]);

        vkEndCommandBuffer(commandBuffers[0]);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        const auto queue = DeviceLocator::GetDevice().GetTransferQueue();

        vkQueueSubmit(queue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(queue);
    }
};

} // namespace estun