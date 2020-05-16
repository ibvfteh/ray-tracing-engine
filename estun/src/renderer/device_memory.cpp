#include "renderer/device_memory.h"
#include "renderer/context/device.h"

estun::DeviceMemory::DeviceMemory(
	const size_t size, 
	const uint32_t memoryTypeBits, 
	const VkMemoryPropertyFlags properties,
	bool flag) 
{
	VkMemoryAllocateFlagsInfo flagsInfo = {};
	flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	if (flag)
	{
		allocInfo.pNext = &flagsInfo;
	}
	allocInfo.memoryTypeIndex = FindMemoryType(memoryTypeBits, properties);

	auto rres = vkAllocateMemory(DeviceLocator::GetLogicalDevice(), &allocInfo, nullptr, &memory);
	VK_CHECK_RESULT(rres, "Failed to allocate memory");
}

estun::DeviceMemory::DeviceMemory(DeviceMemory&& other) noexcept :
	memory(other.memory)
{
	other.memory = nullptr;
}

estun::DeviceMemory::~DeviceMemory()
{
	if (memory != nullptr)
	{
		vkFreeMemory(DeviceLocator::GetLogicalDevice(), memory, nullptr);
		memory = nullptr;
	}
}

void* estun::DeviceMemory::Map(const size_t offset, const size_t size)
{
	void* data;
	VK_CHECK_RESULT(vkMapMemory(DeviceLocator::GetLogicalDevice(), memory, offset, size, 0, &data), "Failed to map memory");

	return data;
}

void estun::DeviceMemory::Unmap()
{
	vkUnmapMemory(DeviceLocator::GetLogicalDevice(), memory);
}

VkDeviceMemory estun::DeviceMemory::GetMemory() const
{
	return memory;
}

uint32_t estun::DeviceMemory::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(DeviceLocator::GetPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (memProperties.memoryTypes[i].propertyFlags == properties)
        {
            return i;
        }
    }

    ES_CORE_ASSERT("Failed to find suitable memory type");
    return 0;
}