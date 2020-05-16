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

    struct VkGeometryInstance
    {
        float transform[12];
        uint32_t instanceCustomIndex : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    class TopLevelAccelerationStructure : public BaseAccelerationStructure, public Descriptable
    {
    public:
        TopLevelAccelerationStructure(const TopLevelAccelerationStructure &) = delete;
        TopLevelAccelerationStructure &operator=(const TopLevelAccelerationStructure &) = delete;
        TopLevelAccelerationStructure &operator=(TopLevelAccelerationStructure &&) = delete;

        TopLevelAccelerationStructure(const std::vector<VkGeometryInstance> &geometryInstances, bool allowUpdate);
        TopLevelAccelerationStructure(TopLevelAccelerationStructure &&other) noexcept;
        virtual ~TopLevelAccelerationStructure();

        DescriptableInfo GetInfo() override;

        const std::vector<VkGeometryInstance> &GeometryInstances() const { return geometryInstances_; }

        void Generate(
            VkCommandBuffer commandBuffer,
            Buffer &scratchBuffer,
            VkDeviceSize scratchOffset,
            DeviceMemory &resultMemory,
            VkDeviceSize resultOffset,
            Buffer &instanceBuffer,
            DeviceMemory &instanceMemory,
            VkDeviceSize instanceOffset,
            bool updateOnly) const;

        static VkGeometryInstance CreateGeometryInstance(
            const BottomLevelAccelerationStructure &bottomLevelAs,
            const glm::mat4 &transform,
            uint32_t instanceId,
            uint32_t hitGroupIndex);

    private:
        std::vector<VkGeometryInstance> geometryInstances_;
    };

} // namespace estun
