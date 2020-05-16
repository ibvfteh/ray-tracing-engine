#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/buffers/vertex.h"
#include "renderer/context/single_time_commands.h"
#include "renderer/ray_tracing/base_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/device_memory.h"
#include "renderer/context/dynamic_functions.h"

namespace
{

VkAccelerationStructureCreateInfoNV GetCreateInfo(const std::vector<VkGeometryNV>& geometries, const bool allowUpdate)
{
		const auto flags = allowUpdate 
			? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV 
			: VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;

		VkAccelerationStructureCreateInfoNV structureInfo = {};
		structureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		structureInfo.pNext = nullptr;
		structureInfo.compactedSize = 0;
		structureInfo.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		structureInfo.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		structureInfo.info.flags = flags;
		structureInfo.info.instanceCount = 0; // The bottom-level AS can only contain explicit geometry, and no instances
		structureInfo.info.geometryCount = static_cast<uint32_t>(geometries.size());
		structureInfo.info.pGeometries = geometries.data();

		return structureInfo;
}

} // namespace

estun::BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(
	const std::vector<VkGeometryNV>& geometries, 
    const bool allowUpdate)
    : BaseAccelerationStructure(GetCreateInfo(geometries, allowUpdate)),
      geometries_(geometries)
{
}

estun::BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(BottomLevelAccelerationStructure &&other) noexcept
    : BaseAccelerationStructure(std::move(other)),
      geometries_(std::move(other.geometries_))
{
}

estun::BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
}

void estun::BottomLevelAccelerationStructure::Generate(
	VkCommandBuffer commandBuffer, 
	Buffer& scratchBuffer,
	const VkDeviceSize scratchOffset,
	DeviceMemory& resultMemory,
	const VkDeviceSize resultOffset,
	const bool updateOnly) const
{
    if (updateOnly && !allowUpdate_)
    {
        ES_CORE_ASSERT("Cannot update readonly structure");
    }

	const VkAccelerationStructureNV previousStructure = updateOnly ? GetStructure() : nullptr;

	VkBindAccelerationStructureMemoryInfoNV bindInfo = {};	
	bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	bindInfo.pNext = nullptr;
	bindInfo.accelerationStructure = GetStructure();
	bindInfo.memory = resultMemory.GetMemory();
	bindInfo.memoryOffset = resultOffset;
	bindInfo.deviceIndexCount = 0;
	bindInfo.pDeviceIndices = nullptr;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryNV(DeviceLocator::GetLogicalDevice(), 1, &bindInfo), "bind acceleration structure");

    const auto flags = allowUpdate_ 
		? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV 
		: VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	
	VkAccelerationStructureInfoNV buildInfo = {};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	buildInfo.pNext = nullptr;
	buildInfo.flags = flags;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	buildInfo.instanceCount = 0;
	buildInfo.geometryCount = static_cast<uint32_t>(geometries_.size());
	buildInfo.pGeometries = geometries_.data();

	FunctionsLocator::GetFunctions().vkCmdBuildAccelerationStructureNV(
		commandBuffer, &buildInfo, nullptr, 0, updateOnly, GetStructure(), previousStructure, scratchBuffer.GetBuffer(), scratchOffset);
}

VkGeometryNV estun::BottomLevelAccelerationStructure::CreateGeometry(
    const Buffer &vertexBuffer,
    const Buffer &indexBuffer,
	const uint32_t vertexOffset, const uint32_t vertexCount,
	const uint32_t indexOffset, const uint32_t indexCount,
	const bool isOpaque)
{
	VkGeometryNV geometry = {};
	geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	geometry.pNext = nullptr;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	geometry.geometry.triangles.pNext = nullptr;
	geometry.geometry.triangles.vertexData = vertexBuffer.GetBuffer();
	geometry.geometry.triangles.vertexOffset = vertexOffset;
	geometry.geometry.triangles.vertexCount = vertexCount;
	geometry.geometry.triangles.vertexStride = sizeof(Vertex);
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	geometry.geometry.triangles.indexData = indexBuffer.GetBuffer();
	geometry.geometry.triangles.indexOffset = indexOffset;
	geometry.geometry.triangles.indexCount = indexCount;
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	geometry.geometry.triangles.transformData = nullptr;
	geometry.geometry.triangles.transformOffset = 0;
	geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	geometry.flags = isOpaque ? VK_GEOMETRY_OPAQUE_BIT_NV : 0;

	return geometry;
}

