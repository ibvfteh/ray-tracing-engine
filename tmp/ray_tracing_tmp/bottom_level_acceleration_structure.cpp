#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/buffers/vertex.h"
#include "renderer/context/single_time_commands.h"
#include "renderer/ray_tracing/base_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/device_memory.h"
#include "renderer/context/dynamic_functions.h"

namespace
{

VkAccelerationStructureCreateInfoKHR GetCreateInfo(const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos, const bool allowUpdate)
{
    const auto flags = allowUpdate
                           ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                           : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; // VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR

    VkAccelerationStructureCreateInfoKHR structureInfo = {};
    structureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    structureInfo.pNext = nullptr;
    structureInfo.compactedSize = 0;
    structureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    structureInfo.flags = flags;
    structureInfo.maxGeometryCount = static_cast<uint32_t>(geometryInfos.size());
    structureInfo.pGeometryInfos = geometryInfos.data();
    structureInfo.deviceAddress = 0;

    return structureInfo;
}

} // namespace

estun::BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(
    const std::vector<VkAccelerationStructureGeometryKHR> &geometries,
    const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos,
    const std::vector<VkAccelerationStructureBuildOffsetInfoKHR> &buildOffsetInfos,
    const bool allowUpdate)
    : BaseAccelerationStructure(GetCreateInfo(geometryInfos, allowUpdate)),
      geometries_(geometries),
      geometryInfos_(geometryInfos),
      buildOffsetInfos_(buildOffsetInfos)
{
}

estun::BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(BottomLevelAccelerationStructure &&other) noexcept
    : BaseAccelerationStructure(std::move(other)),
      geometries_(std::move(other.geometries_)),
      geometryInfos_(std::move(other.geometryInfos_)),
      buildOffsetInfos_(std::move(other.buildOffsetInfos_))
{
}

estun::BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
}

void estun::BottomLevelAccelerationStructure::Generate(
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

    // Bind the acceleration structure descriptor to the actual memory that will contain it
    VkBindAccelerationStructureMemoryInfoKHR bindInfo = {};
    bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
    bindInfo.pNext = nullptr;
    bindInfo.accelerationStructure = GetStructure();
    bindInfo.memory = resultMemory.GetMemory();
    bindInfo.memoryOffset = resultOffset;
    bindInfo.deviceIndexCount = 0;
    bindInfo.pDeviceIndices = nullptr;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkBindAccelerationStructureMemoryKHR(DeviceLocator::GetLogicalDevice(), 1, &bindInfo), "bind acceleration structure");

    // Build the actual bottom-level acceleration structure
    const auto flags = allowUpdate_
                           ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
                           : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    /*
    VkAccelerationStructureInfoNV buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    buildInfo.pNext = nullptr;
    buildInfo.flags = flags;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    buildInfo.instanceCount = 0;
    buildInfo.geometryCount = static_cast<uint32_t>(geometries_.size());
    buildInfo.pGeometries = geometries_.data();
*/
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags = flags;
    buildInfo.update = updateOnly;
    buildInfo.srcAccelerationStructure = previousStructure;
    buildInfo.dstAccelerationStructure = GetStructure();
    buildInfo.geometryArrayOfPointers = VK_TRUE;
    buildInfo.geometryCount = geometries_.size();
    buildInfo.scratchData.deviceAddress = ;

    const VkAccelerationStructureGeometryKHR *pGeometries = geometries_.data();
    buildInfo.ppGeometries = &pGeometries;

    const VkAccelerationStructureBuildOffsetInfoKHR *pOffsets = buildOffsetInfos_.data();

    FunctionsLocator::GetFunctions().vkCmdBuildAccelerationStructureKHR(commandBuffer, 1, &buildInfo, &pOffsets);
}

VkAccelerationStructureGeometryKHR estun::BottomLevelAccelerationStructure::CreateGeometry(
    const Buffer &vertexBuffer,
    const Buffer &indexBuffer,
    bool isOpaque)
{
    VkAccelerationStructureGeometryKHR geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.pNext = nullptr;
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    geometry.geometry.triangles.pNext = nullptr;
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer.GetDeviceAddress();
    geometry.geometry.triangles.vertexStride = sizeof(Vertex);
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry.geometry.triangles.indexData.deviceAddress = indexBuffer.GetDeviceAddress();
    geometry.geometry.triangles.transformData.deviceAddress = 0;
    geometry.flags = isOpaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;

    return geometry;
}

VkAccelerationStructureCreateGeometryTypeInfoKHR estun::BottomLevelAccelerationStructure::CreateGeometryInfo(
    uint32_t primitiveCount, uint32_t vertexCount)
{
    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryInfo.pNext = nullptr;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryInfo.maxPrimitiveCount = primitiveCount;
    geometryInfo.indexType = VK_INDEX_TYPE_UINT32;
    geometryInfo.maxVertexCount = vertexCount;
    geometryInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometryInfo.allowsTransforms = false;

    return geometryInfo;
}

VkAccelerationStructureBuildOffsetInfoKHR estun::BottomLevelAccelerationStructure::CreateBuildOffsetInfo(
    uint32_t primitiveCount, uint32_t primitiveOffset, uint32_t firstVertex)
{
    VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo = {};
    buildOffsetInfo.primitiveCount = primitiveCount;
    buildOffsetInfo.primitiveOffset = primitiveOffset;
    buildOffsetInfo.firstVertex = firstVertex;
    buildOffsetInfo.transformOffset = 0;

    return buildOffsetInfo;
}

VkDeviceAddress estun::BottomLevelAccelerationStructure::GetDeviceAddress() const
{
    VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo = {};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    deviceAddressInfo.pNext = nullptr;
    deviceAddressInfo.accelerationStructure = GetStructure();
    VkDeviceAddress deviceAddress = FunctionsLocator::GetFunctions().vkGetAccelerationStructureDeviceAddressKHR(DeviceLocator::GetLogicalDevice(), &deviceAddressInfo);
    if (deviceAddress == 0)
    {
        ES_CORE_ASSERT("Failed to find device address for bottom level acceleration structure");
    }

    return deviceAddress;
}
