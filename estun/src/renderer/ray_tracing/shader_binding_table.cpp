#include "renderer/ray_tracing//shader_binding_table.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/device.h"
#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/ray_tracing_properties.h"
#include "renderer/device_memory.h"
#include "renderer/context/dynamic_functions.h"

estun::ShaderBindingTable::ShaderBindingTable(
    const std::shared_ptr<RayTracingPipeline> rayTracingPipeline,
    std::vector<uint32_t> programs)
{

    uint32_t groupHandleSize = estun::RayTracingPropertiesLocator::GetProperties().ShaderGroupHandleSize();
    uint32_t shaderBindingTableGroupCount = programs.size();
    sbtSize_ = shaderBindingTableGroupCount * groupHandleSize;

    buffer_.reset(new estun::Buffer(sbtSize_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
    bufferMemory_.reset(new estun::DeviceMemory(buffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)));

    void *dstData = static_cast<uint8_t *>(bufferMemory_->Map(0, sbtSize_));

    estun::FunctionsLocator::GetFunctions().vkGetRayTracingShaderGroupHandlesKHR(
        estun::DeviceLocator::GetLogicalDevice(), rayTracingPipeline->GetPipeline(),
        0, shaderBindingTableGroupCount,
        sbtSize_, dstData);

    bufferMemory_->Unmap();

    rayGenEntrySize_ = groupHandleSize;
    missEntrySize_ = groupHandleSize;
    hitGroupEntrySize_ = groupHandleSize;

    rayGenOffset_ = 0;
    missOffset_ = rayGenEntrySize_;
    hitGroupOffset_ = rayGenEntrySize_ + hitGroupEntrySize_;
}

estun::ShaderBindingTable::~ShaderBindingTable()
{
    buffer_.reset();
    bufferMemory_.reset();
}
