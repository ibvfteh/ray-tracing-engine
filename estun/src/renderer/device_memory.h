#pragma once

#include "renderer/common.h"

namespace estun
{

class DeviceMemory
{
public:
	DeviceMemory(const DeviceMemory &) = delete;
	DeviceMemory &operator=(const DeviceMemory &) = delete;
	DeviceMemory &operator=(DeviceMemory &&) = delete;

	DeviceMemory(size_t size, uint32_t memoryTypeBits, VkMemoryPropertyFlags properties, bool flag = false);
	DeviceMemory(DeviceMemory &&other) noexcept;
	~DeviceMemory();

	void *Map(size_t offset, size_t size);
	void Unmap();

	VkDeviceMemory GetMemory() const;

private:
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	VkDeviceMemory memory;
};

} // namespace estun