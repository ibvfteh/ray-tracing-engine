#pragma once

#include "renderer/common.h"
#include "renderer/ray_tracing/base_acceleration_structure.h"

namespace estun
{

class DeviceMemory;
class Buffer;
class BaseAccelerationStructure;

class BottomLevelAccelerationStructure : public BaseAccelerationStructure
{
public:
    BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure &) = delete;
    BottomLevelAccelerationStructure(BottomLevelAccelerationStructure &&other) noexcept;
    
    BottomLevelAccelerationStructure &operator=(const BottomLevelAccelerationStructure &) = delete;
    BottomLevelAccelerationStructure &operator=(BottomLevelAccelerationStructure &&) = delete;

    BottomLevelAccelerationStructure(
        const std::vector<VkAccelerationStructureGeometryKHR> &geometries,
        const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos,
        const std::vector<VkAccelerationStructureBuildOffsetInfoKHR> &buildOffsetInfos,
        bool allowUpdate);
    ~BottomLevelAccelerationStructure();

    void Generate(
        VkCommandBuffer commandBuffer,
        DeviceMemory &resultMemory,
        VkDeviceSize resultOffset,
        bool updateOnly) const;

    VkDeviceAddress GetDeviceAddress() const;

    static VkAccelerationStructureGeometryKHR CreateGeometry(const Buffer &vertexBuffer, const Buffer &indexBuffer, bool isOpaque);
    static VkAccelerationStructureCreateGeometryTypeInfoKHR CreateGeometryInfo(uint32_t primitiveCount, uint32_t vertexCount);
    static VkAccelerationStructureBuildOffsetInfoKHR CreateBuildOffsetInfo(uint32_t primitiveCount, uint32_t primitiveOffset, uint32_t firstVertex);

private:
    std::vector<VkAccelerationStructureGeometryKHR> geometries_;
    std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> geometryInfos_;
    std::vector<VkAccelerationStructureBuildOffsetInfoKHR> buildOffsetInfos_;
};

} // namespace estun
