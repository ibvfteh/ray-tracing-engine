#include "renderer/buffers/buffer.h"
#include "renderer/context/single_time_commands.h"
#include "renderer/context/device.h"
#include "renderer/device_memory.h"
#include "renderer/context/command_pool.h"
#include "renderer/context/dynamic_functions.h"

estun::Buffer::Buffer(const size_t size, const VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(DeviceLocator::GetLogicalDevice(), &bufferInfo, nullptr, &buffer), "Failed to create buffer");
}

estun::Buffer::~Buffer()
{
    if (buffer != nullptr)
    {
        vkDestroyBuffer(DeviceLocator::GetLogicalDevice(), buffer, nullptr);
        buffer = nullptr;
    }
}

estun::DeviceMemory estun::Buffer::AllocateMemory(const VkMemoryPropertyFlags properties, bool flag)
{
    const auto requirements = GetMemoryRequirements();
    DeviceMemory memory(requirements.size, requirements.memoryTypeBits, properties, flag);

    VK_CHECK_RESULT(vkBindBufferMemory(DeviceLocator::GetLogicalDevice(), buffer, memory.GetMemory(), 0), "Failed to bind buffer memory");

    return memory;
}

VkMemoryRequirements estun::Buffer::GetMemoryRequirements() const
{
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(DeviceLocator::GetLogicalDevice(), buffer, &requirements);
    return requirements;
}

void estun::Buffer::CopyFrom(const Buffer &src, VkDeviceSize size)
{
    SingleTimeCommands::SubmitTransfer(CommandPoolLocator::GetTransferPool(), [&](VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, src.GetBuffer(), GetBuffer(), 1, &copyRegion);
    });
}

VkDeviceAddress estun::Buffer::GetDeviceAddress() const
{
    VkBufferDeviceAddressInfo info;
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.pNext = nullptr;
    info.buffer = buffer;
    VkDeviceAddress deviceAddress = vkGetBufferDeviceAddress(DeviceLocator::GetLogicalDevice(), &info);
    return deviceAddress;
}

VkBuffer estun::Buffer::GetBuffer() const
{
    return buffer;
}

void estun::Buffer::BufferMemoryBarrier(VkCommandBuffer commandBuffer, const Buffer &buffer, bool type)
{
    if (type)
    {
        VkBufferMemoryBarrier bufferMemoryBarrier = {};
        bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferMemoryBarrier.pNext = NULL;
        bufferMemoryBarrier.srcAccessMask = 0;
        bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bufferMemoryBarrier.srcQueueFamilyIndex = DeviceLocator::GetDevice().GetQueueFamilyIndices().transferFamily.value();
        bufferMemoryBarrier.dstQueueFamilyIndex = DeviceLocator::GetDevice().GetQueueFamilyIndices().computeFamily.value();
        bufferMemoryBarrier.buffer = buffer.GetBuffer();
        bufferMemoryBarrier.offset = 0;
        bufferMemoryBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
    }
    else
    {

        VkBufferMemoryBarrier bufferMemoryBarrier = {};
        bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferMemoryBarrier.pNext = NULL;
        bufferMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bufferMemoryBarrier.dstAccessMask = 0;
        bufferMemoryBarrier.srcQueueFamilyIndex = DeviceLocator::GetDevice().GetQueueFamilyIndices().computeFamily.value();
        bufferMemoryBarrier.dstQueueFamilyIndex = DeviceLocator::GetDevice().GetQueueFamilyIndices().transferFamily.value();
        bufferMemoryBarrier.buffer = buffer.GetBuffer();
        bufferMemoryBarrier.offset = 0;
        bufferMemoryBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
    }
}