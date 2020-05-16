#include "renderer/ray_tracing/ray_tracing_properties.h"
#include "renderer/context/device.h"

estun::RayTracingProperties::RayTracingProperties()
{

	VkPhysicalDeviceRayTracingPropertiesKHR rayTracingProperties;
	rayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR;
	rayTracingProperties.pNext = nullptr;

	VkPhysicalDeviceProperties2 deviceProperties2;
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingProperties;
	vkGetPhysicalDeviceProperties2(DeviceLocator::GetPhysicalDevice(), &deviceProperties2);
}

estun::RayTracingProperties *estun::RayTracingPropertiesLocator::props_ = nullptr;