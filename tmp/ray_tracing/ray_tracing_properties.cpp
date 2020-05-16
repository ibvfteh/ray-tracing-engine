#include "renderer/ray_tracing/ray_tracing_properties.h"
#include "renderer/context/device.h"

estun::RayTracingProperties::RayTracingProperties() 
{
	props_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;

	VkPhysicalDeviceProperties2 props = {};
	props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	props.pNext = &props_;
	vkGetPhysicalDeviceProperties2(DeviceLocator::GetPhysicalDevice(), &props);
}
