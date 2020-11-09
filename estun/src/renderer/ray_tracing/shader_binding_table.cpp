#include "renderer/ray_tracing//shader_binding_table.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/device.h"
#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/ray_tracing_properties.h"
#include "renderer/device_memory.h"
#include "renderer/context/dynamic_functions.h"
#include "core/core.h"

estun::ShaderBindingTable::ShaderBindingTable(
    const std::shared_ptr<RayTracingPipeline> rayTracingPipeline)
{

    uint32_t groupHandleSize = estun::RayTracingPropertiesLocator::GetProperties().ShaderGroupHandleSize();
    uint32_t groupHandlealignment = estun::RayTracingPropertiesLocator::GetProperties().ShaderGroupBaseAlignment();
    uint32_t shaderBindingTableGroupCount = rayTracingPipeline->GetGroupCount();
    sbtSize_ = static_cast<size_t>(shaderBindingTableGroupCount) * groupHandlealignment;

    buffer_.reset(new estun::Buffer(sbtSize_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
    bufferMemory_.reset(new estun::DeviceMemory(buffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)));

    auto *dstData = static_cast<uint8_t *>(bufferMemory_->Map(0, sbtSize_));

    std::vector<uint8_t> shaderHandleStorage(sbtSize_);
    estun::FunctionsLocator::GetFunctions().vkGetRayTracingShaderGroupHandlesKHR(
        estun::DeviceLocator::GetLogicalDevice(), rayTracingPipeline->GetPipeline(),
        0, shaderBindingTableGroupCount,
        sbtSize_, shaderHandleStorage.data());

    for (uint32_t i = 0; i < shaderBindingTableGroupCount; i++)
    {
        memcpy(dstData, shaderHandleStorage.data() + static_cast<size_t>(i) * groupHandleSize, groupHandleSize);
        dstData += groupHandlealignment;
    }

    bufferMemory_->Unmap();

    rayGenEntrySize_ = groupHandleSize;
    missEntrySize_ = groupHandleSize;
    hitGroupEntrySize_ = groupHandleSize;
    callEntrySize_ = groupHandleSize;

    rayGenOffset_ = 0;
    missOffset_ = groupHandlealignment;
    hitGroupOffset_ = static_cast<size_t>(groupHandlealignment) * 2;
    callOffset_ = static_cast<size_t>(groupHandlealignment) * 3;
}

estun::ShaderBindingTable::~ShaderBindingTable()
{
    buffer_.reset();
    bufferMemory_.reset();
}
