#pragma once

#include "renderer/common.h"

namespace estun
{

    class BottomLevelAccelerationStructure;
    class TopLevelAccelerationStructure;
    class Buffer;
    class VertexBuffer;
    class IndexBuffer;
    class DeviceMemory;
    class Model;

    class AccelerationStructureManager
    {
    public:
        AccelerationStructureManager(const AccelerationStructureManager &) = delete;
        AccelerationStructureManager &operator=(const AccelerationStructureManager &) = delete;
        AccelerationStructureManager &operator=(AccelerationStructureManager &&) = delete;
        AccelerationStructureManager(){};

        void Submit(std::vector<Model> &models, VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, bool allowUpdate);
        void DeleteAccelerationStructures();

        //void Update(std::vector<Model> &models, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, bool allowUpdate);

        AccelerationStructureManager(AccelerationStructureManager &&other) noexcept;
        ~AccelerationStructureManager();

        std::vector<BottomLevelAccelerationStructure> &GetBLAS() { return bottomAs_; }
        std::vector<TopLevelAccelerationStructure> &GetTLAS() { return topAs_; }

    private:
        bool allowUpdate_;

        void CreateBottomLevelStructures(VkCommandBuffer commandBuffer, const std::vector<Model> &models, VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, bool allowUpdate);
        void CreateTopLevelStructures(VkCommandBuffer commandBuffer, const std::vector<Model> &models, bool allowUpdate);

        //void UpdateBottomLevelStructures(VkCommandBuffer commandBuffer, const std::vector<Model> &models, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, bool allowUpdate);
        //void UpdateTopLevelStructures(VkCommandBuffer commandBuffer, const std::vector<Model> &models, bool allowUpdate);

        std::vector<BottomLevelAccelerationStructure> bottomAs_;
        std::unique_ptr<Buffer> bottomBuffer_;
        std::unique_ptr<DeviceMemory> bottomBufferMemory_;
        std::unique_ptr<Buffer> bottomScratchBuffer_;
        std::unique_ptr<DeviceMemory> bottomScratchBufferMemory_;

        std::vector<TopLevelAccelerationStructure> topAs_;
        std::unique_ptr<Buffer> topBuffer_;
        std::unique_ptr<DeviceMemory> topBufferMemory_;
        std::unique_ptr<Buffer> topScratchBuffer_;
        std::unique_ptr<DeviceMemory> topScratchBufferMemory_;
        std::unique_ptr<Buffer> instancesBuffer_;
        std::unique_ptr<DeviceMemory> instancesBufferMemory_;
    };

} // namespace estun