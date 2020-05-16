#include "renderer/ray_tracing/acceleration_structure_manager.h"
#include "renderer/context/single_time_commands.h"
#include "renderer/ray_tracing/bottom_level_acceleration_structure.h"
#include "renderer/ray_tracing/top_level_acceleration_structure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/device_memory.h"
#include "renderer/model.h"

namespace
{
    estun::BaseAccelerationStructure::MemoryRequirements GetTotalRequirements(
        const std::vector<estun::BaseAccelerationStructure::MemoryRequirements> &requirements)
    {
        estun::BaseAccelerationStructure::MemoryRequirements total{};

        for (const auto &req : requirements)
        {
            total.result.size += req.result.size;
            total.build.size += req.build.size;
            total.update.size += req.update.size;
        }

        return total;
    }

} // namespace

void estun::AccelerationStructureManager::Submit(
    std::vector<estun::Model> &models,
    estun::VertexBuffer *vertexBuffer,
    estun::IndexBuffer *indexBuffer,
    bool allowUpdate)
{
    SingleTimeCommands::SubmitCompute(CommandPoolLocator::GetComputePool(), [this, models, vertexBuffer, indexBuffer, allowUpdate](VkCommandBuffer commandBuffer) {
        CreateBottomLevelStructures(commandBuffer, models, vertexBuffer, indexBuffer, allowUpdate);
        BaseAccelerationStructure::ASMemoryBarrier(commandBuffer);
        CreateTopLevelStructures(commandBuffer, models, allowUpdate);
    });

    topScratchBuffer_.reset();
    topScratchBufferMemory_.reset();
    bottomScratchBuffer_.reset();
    bottomScratchBufferMemory_.reset();
}

void estun::AccelerationStructureManager::DeleteAccelerationStructures()
{
    topAs_.clear();
    instancesBuffer_.reset();
    instancesBufferMemory_.reset();
    topScratchBuffer_.reset();
    topScratchBufferMemory_.reset();
    topBuffer_.reset();
    topBufferMemory_.reset();

    bottomAs_.clear();
    bottomScratchBuffer_.reset();
    bottomScratchBufferMemory_.reset();
    bottomBuffer_.reset();
    bottomBufferMemory_.reset();
}

estun::AccelerationStructureManager::~AccelerationStructureManager()
{
    DeleteAccelerationStructures();
}
/*
void estun::AccelerationStructureManager::Update(
    std::vector<estun::Model> &models,
    estun::VertexBuffer *vertexBuffer,
    estun::IndexBuffer *indexBuffer,
    bool allowUpdate)
{
    if (!allowUpdate_)
    {
        ES_CORE_ASSERT("Cannot update readonly acceleration structure");
    }

    bottomAs_.clear();
    topAs_.clear();

    SingleTimeCommands::SubmitCompute(CommandPoolLocator::GetComputePool(), [this, models, vertexBuffer, indexBuffer, allowUpdate](VkCommandBuffer commandBuffer) {
        UpdateBottomLevelStructures(commandBuffer, models, vertexBuffer, indexBuffer, allowUpdate);
        BaseAccelerationStructure::ASMemoryBarrier(commandBuffer);
        UpdateTopLevelStructures(commandBuffer, models, allowUpdate);
    });
}
*/
void estun::AccelerationStructureManager::CreateBottomLevelStructures(
    VkCommandBuffer commandBuffer,
    const std::vector<estun::Model> &models,
    estun::VertexBuffer *vertexBuffer,
    estun::IndexBuffer *indexBuffer,
    bool allowUpdate)
{
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    std::vector<BaseAccelerationStructure::MemoryRequirements> requirements;

    for (const auto &model : models)
    {
        const auto vertexCount = static_cast<uint32_t>(model.GetVertices().size());
        const auto indexCount = static_cast<uint32_t>(model.GetIndices().size());
        const std::vector<VkGeometryNV> geometries =
            {
                BottomLevelAccelerationStructure::CreateGeometry(
                    vertexBuffer->GetBuffer(), indexBuffer->GetBuffer(),
                    vertexOffset, vertexCount,
                    indexOffset, indexCount,
                    true)};

        bottomAs_.emplace_back(geometries, false);
        requirements.push_back(bottomAs_.back().GetMemoryRequirements());

        vertexOffset += vertexCount * sizeof(Vertex);
        indexOffset += indexCount * sizeof(uint32_t);
    }

    const auto total = GetTotalRequirements(requirements);

    bottomBuffer_.reset(new Buffer(total.result.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV));
    bottomBufferMemory_.reset(new DeviceMemory(bottomBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    bottomScratchBuffer_.reset(new Buffer(total.build.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV));
    bottomScratchBufferMemory_.reset(new DeviceMemory(bottomScratchBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    VkDeviceSize resultOffset = 0;
    VkDeviceSize scratchOffset = 0;

    for (size_t i = 0; i != bottomAs_.size(); ++i)
    {
        bottomAs_[i].Generate(commandBuffer, *bottomScratchBuffer_, scratchOffset, *bottomBufferMemory_, resultOffset, false);
        resultOffset += requirements[i].result.size;
        scratchOffset += requirements[i].build.size;
    }
}

void estun::AccelerationStructureManager::CreateTopLevelStructures(
    VkCommandBuffer commandBuffer,
    const std::vector<estun::Model> &models,
    bool allowUpdate)
{
    std::vector<VkGeometryInstance> geometryInstances;
    std::vector<BaseAccelerationStructure::MemoryRequirements> requirements;

    // Hit group 0: triangles
    // Hit group 1: procedurals

    uint32_t instanceId = 0;

    for (const auto &model : models)
    {
        geometryInstances.push_back(TopLevelAccelerationStructure::CreateGeometryInstance(bottomAs_[instanceId], glm::mat4(1), instanceId, 0));
        instanceId++;
    }

    topAs_.emplace_back(geometryInstances, false);
    requirements.push_back(topAs_.back().GetMemoryRequirements());

    // Allocate the structure memory.
    const auto total = GetTotalRequirements(requirements);

    topBuffer_.reset(new Buffer(total.result.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV));
    topBufferMemory_.reset(new DeviceMemory(topBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    topScratchBuffer_.reset(new Buffer(total.build.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV));
    topScratchBufferMemory_.reset(new DeviceMemory(topScratchBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    const size_t instancesBufferSize = sizeof(VkGeometryInstance) * geometryInstances.size();
    instancesBuffer_.reset(new Buffer(instancesBufferSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV));
    instancesBufferMemory_.reset(new DeviceMemory(instancesBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));

    topAs_[0].Generate(commandBuffer, *topScratchBuffer_, 0, *topBufferMemory_, 0, *instancesBuffer_, *instancesBufferMemory_, 0, false);
}
/*
void estun::AccelerationStructureManager::UpdateBottomLevelStructures(
    VkCommandBuffer commandBuffer,
    const std::vector<estun::Model> &models,
    estun::VertexBuffer *vertexBuffer,
    estun::IndexBuffer *indexBuffer,
    bool allowUpdate)
{
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    std::vector<MemoryRequirements> requirements;

    for (const auto &model : models)
    {
        const auto vertexCount = static_cast<uint32_t>(model.SizeOfVertices());
        const auto indexCount = static_cast<uint32_t>(model.SizeOfIndices());
        const std::vector<VkAccelerationStructureGeometryKHR> geometries = {
            BottomLevelAccelerationStructure::CreateGeometry(
                vertexBuffer->GetBuffer(),
                indexBuffer->GetBuffer(),
                vertexOffset,
                vertexCount,
                indexOffset,
                indexCount,
                true)};

        bottomAs_.emplace_back(geometries, false);
        requirements.push_back(bottomAs_.back().GetMemoryRequirements());

        vertexOffset += vertexCount * sizeof(Vertex);
        indexOffset += indexCount * sizeof(uint32_t);
    }

    const auto total = GetTotalRequirements(requirements);

    bottomBuffer_.reset(new Buffer(total.result.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR));
    bottomBufferMemory_.reset(new DeviceMemory(bottomBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    bottomScratchBuffer_.reset(new Buffer(total.build.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR));
    bottomScratchBufferMemory_.reset(new DeviceMemory(bottomScratchBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    VkDeviceSize resultOffset = 0;
    VkDeviceSize scratchOffset = 0;

    for (size_t i = 0; i != bottomAs_.size(); ++i)
    {
        bottomAs_[i].Generate(commandBuffer, *bottomScratchBuffer_, scratchOffset, *bottomBufferMemory_, resultOffset, true);
        resultOffset += requirements[i].result.size;
        scratchOffset += requirements[i].build.size;
    }
}

void estun::AccelerationStructureManager::UpdateTopLevelStructures(
    VkCommandBuffer commandBuffer,
    const std::vector<estun::Model> &models,
    bool allowUpdate)
{
    std::vector<VkGeometryInstance> geometryInstances;
    std::vector<MemoryRequirements> requirements;

    uint32_t instanceId = 0;

    for (const auto &model : models)
    {
        geometryInstances.push_back(TopLevelAccelerationStructure::CreateGeometryInstance(bottomAs_[instanceId], glm::mat4(1), instanceId, 0));
        instanceId++;
    }

    topAs_.emplace_back(geometryInstances, false);
    requirements.push_back(topAs_.back().GetMemoryRequirements());

    const auto total = GetTotalRequirements(requirements);

    topBuffer_.reset(new Buffer(total.result.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR));
    topBufferMemory_.reset(new DeviceMemory(topBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    topScratchBuffer_.reset(new Buffer(total.build.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR));
    topScratchBufferMemory_.reset(new DeviceMemory(topScratchBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    const size_t instancesBufferSize = sizeof(VkGeometryInstance) * geometryInstances.size();
    instancesBuffer_.reset(new Buffer(instancesBufferSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR));
    instancesBufferMemory_.reset(new DeviceMemory(instancesBuffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));

    topAs_[0].Generate(commandBuffer, *topScratchBuffer_, 0, *topBufferMemory_, 0, *instancesBuffer_, *instancesBufferMemory_, 0, false);
}
*/