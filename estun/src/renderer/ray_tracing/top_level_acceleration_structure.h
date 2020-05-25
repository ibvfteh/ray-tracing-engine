#pragma once

#include "renderer/common.h"
#include "renderer/material/descriptable.h"

namespace estun
{

    class Buffer;
    class Model;
    class BLAS;
    class DeviceMemory;

    class TLAS : public Descriptable
    {
    public:
        TLAS(const TLAS &) = delete;
        TLAS(TLAS &&) = delete;

        TLAS &operator=(const TLAS &) = delete;
        TLAS &operator=(TLAS &&) = delete;

        TLAS(std::vector<std::shared_ptr<estun::BLAS>> blases);
        ~TLAS();

        VkMemoryRequirements GetBufferMemoryRequirements(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type);

        DescriptableInfo GetInfo() override;

        uint32_t GetBufferSize(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type);

    private:
        std::shared_ptr<DeviceMemory> tlasMemory_;

        VkDeviceAddress deviceAddress_;
        uint32_t buildScratchSize_;
        uint32_t objectSize_;
        VkAccelerationStructureKHR accelerationStructure_;

        std::vector<VkAccelerationStructureBuildOffsetInfoKHR *> buildOffsets_;
        std::vector<VkAccelerationStructureGeometryKHR> accelerationGeometries_;
    };

} // namespace estun