#pragma once

#include "renderer/common.h"

#include "includes/glm.h"
#include "renderer/ray_tracing/base_acceleration_structure.h"
#include "renderer/material/descriptable.h"

namespace estun
{

    class BottomLevelAccelerationStructure;
    class Buffer;
    class DeviceMemory;

    class TopLevelAccelerationStructure : public BaseAccelerationStructure, public Descriptable
    {
    public:
        TopLevelAccelerationStructure(const TopLevelAccelerationStructure &) = delete;
        TopLevelAccelerationStructure &operator=(const TopLevelAccelerationStructure &) = delete;
        TopLevelAccelerationStructure &operator=(TopLevelAccelerationStructure &&) = delete;

        TopLevelAccelerationStructure(
            //const std::vector<VkAccelerationStructureGeometryKHR> &geometries,
            const std::vector<VkAccelerationStructureInstanceKHR> &geometryInstances,
            const std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> &geometryInfos,
            //const std::vector<VkAccelerationStructureBuildOffsetInfoKHR> &buildOffsetInfos,
            bool allowUpdate);
        TopLevelAccelerationStructure(TopLevelAccelerationStructure &&other) noexcept;
        virtual ~TopLevelAccelerationStructure();

        DescriptableInfo GetInfo() override;

        //const std::vector<VkAccelerationStructureGeometryKHR> &GetGeometries() const { return geometries_; }
        const std::vector<VkAccelerationStructureInstanceKHR> &GetGeometryInstances() const { return geometryInstances_; }

        void Generate(
            VkCommandBuffer commandBuffer,
            DeviceMemory &resultMemory,
            VkDeviceSize resultOffset,
            bool updateOnly) const;

        static VkAccelerationStructureCreateGeometryTypeInfoKHR CreateGeometryInfo(uint32_t instanceCount);
        
        VkAccelerationStructureGeometryKHR CreateGeometry(
            std::vector<VkAccelerationStructureInstanceKHR> geometryInstances) const;
        static VkAccelerationStructureInstanceKHR CreateGeometryInstance(
            const BottomLevelAccelerationStructure &bottomLevelAs,
            const glm::mat4 &transform,
            uint32_t instanceId,
            uint32_t hitGroupIndex);
        static VkAccelerationStructureBuildOffsetInfoKHR CreateBuildOffsetInfo(
            uint32_t primitiveCount, uint32_t primitiveOffset);

    private:
        std::vector<VkAccelerationStructureInstanceKHR> geometryInstances_;
        //std::vector<VkAccelerationStructureBuildOffsetInfoKHR> buildOffsetInfos_;
        //std::vector<VkAccelerationStructureGeometryKHR> geometries_;
    };

} // namespace estun
