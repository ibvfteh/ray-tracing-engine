#pragma once

#include "renderer/common.h"
#include "glm/glm.hpp"

namespace estun
{

    class VertexBuffer;
    class IndexBuffer;
    class Buffer;
    class Model;
    class DeviceMemory;

    class BLAS
    {
    public:
        BLAS(const BLAS &) = delete;
        BLAS(BLAS &&) = delete;

        BLAS &operator=(const BLAS &) = delete;
        BLAS &operator=(BLAS &&) = delete;

        BLAS(
            std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer,
            uint32_t vertexCount, uint32_t indexCount, 
            uint32_t vertexOffset, uint32_t indexOffset);
        ~BLAS();

        VkMemoryRequirements GetBufferMemoryRequirements(VkAccelerationStructureKHR accelerationStructure, VkAccelerationStructureMemoryRequirementsTypeKHR type);

        void Generate(std::shared_ptr<DeviceMemory> blasesMemory, uint32_t blasOffset);

        uint32_t GetSize() { return objectSize_; };
        VkDeviceAddress GetDeviceAddress();
        uint32_t GetHitGroup() { return hitGroup_; };
        glm::mat4* GetTransformMatrix() { return &transform_; };
        VkAccelerationStructureKHR GetStructure() { return accelerationStructure_; };

        static std::vector<std::shared_ptr<BLAS>> CreateBlases(std::vector<std::shared_ptr<Model>> models, std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer);

    private:
        std::shared_ptr<DeviceMemory> blasMemory_;

        uint32_t buildScratchSize_;
        uint32_t objectSize_;
        uint32_t hitGroup_ = 0;
        glm::mat4 transform_ = glm::mat4(1.0f);
        VkAccelerationStructureKHR accelerationStructure_;

        std::vector<VkAccelerationStructureBuildOffsetInfoKHR *> buildOffsets_;
        std::vector<VkAccelerationStructureGeometryKHR> accelerationGeometries_;
    };

} // namespace estun