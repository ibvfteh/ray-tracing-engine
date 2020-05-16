#include "renderer/ray_tracing/top_level_acceleration_structure.h"
#include "renderer/buffers/vertex.h"
#include "renderer/context/single_time_commands.h"
#include "renderer/ray_tracing/base_acceleration_structure.h"
#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/device_memory.h"
#include "renderer/material/descriptable.h"
#include <cstring>
#include "renderer/context/dynamic_functions.h"

namespace
{

VkAccelerationStructureCreateInfoKHR GetCreateInfo(const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos, const bool allowUpdate)
{
    const auto flags = allowUpdate
                           ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                           : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

    VkAccelerationStructureCreateInfoKHR structureInfo = {};
    structureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    structureInfo.pNext = nullptr;
    structureInfo.compactedSize = 0;
    structureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    structureInfo.flags = flags;
    structureInfo.maxGeometryCount = 1;
    structureInfo.pGeometryInfos = geometryInfos.data();
    structureInfo.deviceAddress = 0;

    return structureInfo;
}

} // namespace

estun::TopLevelAccelerationStructure::TopLevelAccelerationStructure(
    const std::vector<VkAccelerationStructureInstanceKHR> &geometryInstances,
    const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos,
    //const std::vector<VkAccelerationStructureBuildOffsetInfoKHR> &buildOffsetInfos,
    const bool allowUpdate)
    : BaseAccelerationStructure(GetCreateInfo(geometryInfos, allowUpdate)),
      geometryInstances_(geometryInstances)
{
}

estun::TopLevelAccelerationStructure::TopLevelAccelerationStructure(TopLevelAccelerationStructure &&other) noexcept
    : BaseAccelerationStructure(std::move(other)),
      geometryInstances_(std::move(other.geometryInstances_))
{
}

estun::TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
}

void estun::TopLevelAccelerationStructure::Generate(
    VkCommandBuffer commandBuffer,
    DeviceMemory &resultMemory,
    const VkDeviceSize resultOffset,
    const bool updateOnly) const
{
    if (updateOnly && !allowUpdate_)
    {
        ES_CORE_ASSERT("Cannot update readonly structure");
    }

    const VkAccelerationStructureKHR previousStructure = updateOnly ? GetStructure() : 0;

    VkBindAccelerationStructureMemoryInfoKHR bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
    bindInfo.pNext = nullptr;
    bindInfo.accelerationStructure = GetStructure();
    bindInfo.memory = resultMemory.GetMemory();
    bindInfo.memoryOffset = resultOffset;
    bindInfo.deviceIndexCount = 0;
    bindInfo.pDeviceIndices = nullptr;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryKHR(DeviceLocator::GetLogicalDevice(), 1, &bindInfo), "bind acceleration structure");

    const auto flags = allowUpdate_
                           ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV
                           : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;

    const VkAccelerationStructureGeometryKHR geometries = CreateGeometry(geometryInstances_);
    const VkAccelerationStructureGeometryKHR *pGeometries = &geometries;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags = flags;
    buildInfo.update = updateOnly;
    buildInfo.srcAccelerationStructure = previousStructure;
    buildInfo.dstAccelerationStructure = GetStructure();
    buildInfo.geometryArrayOfPointers = VK_FALSE;
    buildInfo.geometryCount = 1;
    buildInfo.ppGeometries = &pGeometries;

    const VkAccelerationStructureBuildOffsetInfoKHR offsets = CreateBuildOffsetInfo(geometryInstances_.size(), 0);
    const VkAccelerationStructureBuildOffsetInfoKHR *pOffsets = &offsets;

    FunctionsLocator::GetFunctions().vkCmdBuildAccelerationStructureKHR(commandBuffer, 1, &buildInfo, &pOffsets);
}

VkAccelerationStructureCreateGeometryTypeInfoKHR estun::TopLevelAccelerationStructure::CreateGeometryInfo(
    uint32_t instanceCount)
{
    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryInfo.pNext = nullptr;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometryInfo.maxPrimitiveCount = instanceCount;

    return geometryInfo;
}

VkAccelerationStructureGeometryKHR estun::TopLevelAccelerationStructure::CreateGeometry(
    std::vector<VkAccelerationStructureInstanceKHR> geometryInstances) const
{
    VkAccelerationStructureGeometryKHR geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.pNext = nullptr;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometry.geometry.instances.pNext = nullptr;
    geometry.geometry.instances.arrayOfPointers = VK_FALSE;
    geometry.geometry.instances.data.hostAddress = geometryInstances.data();
    geometry.flags = 0;

    return geometry;
}

VkAccelerationStructureInstanceKHR estun::TopLevelAccelerationStructure::CreateGeometryInstance(
    const estun::BottomLevelAccelerationStructure &bottomLevelAs,
    const glm::mat4 &transform,
    const uint32_t instanceId,
    const uint32_t hitGroupIndex)
{
    VkAccelerationStructureInstanceKHR geometryInstance = {};
    std::memcpy(&geometryInstance.transform, &transform, sizeof(glm::mat4)); //sizeof() бы приделать
    geometryInstance.instanceCustomIndex = instanceId;
    geometryInstance.mask = 0xFF;
    geometryInstance.instanceShaderBindingTableRecordOffset = hitGroupIndex;
    geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // Disable culling - more fine control could be provided by the application
    geometryInstance.accelerationStructureReference = bottomLevelAs.GetDeviceAddress();

    return geometryInstance;
}

VkAccelerationStructureBuildOffsetInfoKHR estun::TopLevelAccelerationStructure::CreateBuildOffsetInfo(
    uint32_t primitiveCount, uint32_t primitiveOffset)
{
    VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo = {};
    buildOffsetInfo.primitiveCount = primitiveCount;
    buildOffsetInfo.primitiveOffset = primitiveOffset;
    buildOffsetInfo.firstVertex = 0;
    buildOffsetInfo.transformOffset = 0;

    return buildOffsetInfo;
}

estun::DescriptableInfo estun::TopLevelAccelerationStructure::GetInfo()
{
    const auto accelerationStructureHandle = GetStructure();
    VkWriteDescriptorSetAccelerationStructureKHR structureInfo = {};
    structureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    structureInfo.pNext = nullptr;
    structureInfo.accelerationStructureCount = 1;
    structureInfo.pAccelerationStructures = &accelerationStructureHandle;

    DescriptableInfo info;
    info.asI = structureInfo;

    return info;
}
