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

        BottomLevelAccelerationStructure(const std::vector<VkGeometryNV> &geometries, bool allowUpdate);
        ~BottomLevelAccelerationStructure();

        void Generate(
            VkCommandBuffer commandBuffer,
            Buffer &scratchBuffer,
            VkDeviceSize scratchOffset,
            DeviceMemory &resultMemory,
            VkDeviceSize resultOffset,
            bool updateOnly) const;

        static VkGeometryNV CreateGeometry(
            const Buffer &vertexBuffer, const Buffer &indexBuffer,
            uint32_t vertexOffset, uint32_t vertexCount,
            uint32_t indexOffset, uint32_t indexCount,
            bool isOpaque);

    private:
        std::vector<VkGeometryNV> geometries_;
    };

} // namespace estun
