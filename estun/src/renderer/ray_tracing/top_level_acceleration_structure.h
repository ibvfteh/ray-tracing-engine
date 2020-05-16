#pragma once

#include "renderer/common.h"
#include "renderer/material/descriptable.h"

namespace estun
{

    class VertexBuffer;
    class IndexBuffer;
    class Buffer;
    class Model;
    class BLAS;

    class TLAS : public Descriptable
    {
    public:
        TLAS(const TLAS &) = delete;
        TLAS(TLAS &&) = delete;

        TLAS &operator=(const TLAS &) = delete;
        TLAS &operator=(TLAS &&) = delete;

        TLAS(std::vector<std::shared_ptr<estun::BLAS>> blases);
        ~TLAS();

        DescriptableInfo GetInfo() override;

        uint32_t GetBufferSize(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type);

    private:
        VkDeviceAddress deviceAddress_;
        uint32_t buildScratchSize_;
        uint32_t objectSize_;
        VkAccelerationStructureKHR accelerationStructure_;

        std::vector<VkAccelerationStructureBuildOffsetInfoKHR *> buildOffsets_;
        std::vector<VkAccelerationStructureGeometryKHR> accelerationGeometries_;
    };

} // namespace estun