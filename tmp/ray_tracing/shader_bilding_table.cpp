#include "renderer/ray_tracing//shader_bilding_table.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/device.h"
#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/ray_tracing_properties.h"
#include "renderer/device_memory.h"
#include "renderer/context/dynamic_functions.h"

namespace
{

size_t RoundUp(size_t size, size_t powerOf2Alignment)
{
    return (size + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
}

size_t GetEntrySize(const estun::RayTracingProperties &rayTracingProperties, const std::vector<estun::ShaderBindingTable::Entry> &entries)
{
    size_t maxArgs = 0;

    for (const auto &entry : entries)
    {
        maxArgs = std::max(maxArgs, entry.InlineData.size());
    }

    return RoundUp(rayTracingProperties.ShaderGroupHandleSize() + maxArgs, 16);
}

size_t CopyShaderData(
    uint8_t *const dst,
    const estun::RayTracingProperties &rayTracingProperties,
    const std::vector<estun::ShaderBindingTable::Entry> &entries,
    const size_t entrySize,
    const uint8_t *const shaderHandleStorage)
{
    const auto handleSize = rayTracingProperties.ShaderGroupHandleSize();

    uint8_t *pDst = dst;

    for (const auto &entry : entries)
    {
        std::memcpy(pDst, shaderHandleStorage + entry.GroupIndex * handleSize, handleSize);
        std::memcpy(pDst + handleSize, entry.InlineData.data(), entry.InlineData.size());

        pDst += entrySize;
    }

    return entries.size() * entrySize;
}

} // namespace

estun::ShaderBindingTable::ShaderBindingTable(
    const RayTracingPipeline &rayTracingPipeline,
    const RayTracingProperties &rayTracingProperties,
    const std::vector<Entry> &rayGenPrograms,
    const std::vector<Entry> &missPrograms,
    const std::vector<Entry> &hitGroups)
    : rayGenEntrySize_(GetEntrySize(rayTracingProperties, rayGenPrograms)),
      missEntrySize_(GetEntrySize(rayTracingProperties, missPrograms)),
      hitGroupEntrySize_(GetEntrySize(rayTracingProperties, hitGroups)),
      rayGenOffset_(0),
      missOffset_(rayGenPrograms.size() * rayGenEntrySize_),
      hitGroupOffset_(missOffset_ + missPrograms.size() * missEntrySize_)
{
    const size_t sbtSize =
        rayGenPrograms.size() * rayGenEntrySize_ +
        missPrograms.size() * missEntrySize_ +
        hitGroups.size() * hitGroupEntrySize_;

    buffer_.reset(new Buffer(sbtSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
    bufferMemory_.reset(new DeviceMemory(buffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)));

    const uint32_t handleSize = rayTracingProperties.ShaderGroupHandleSize();
    const size_t groupCount = rayGenPrograms.size() + missPrograms.size() + hitGroups.size();
    std::vector<uint8_t> shaderHandleStorage(groupCount * handleSize);

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkGetRayTracingShaderGroupHandlesNV(
              DeviceLocator::GetLogicalDevice(),
              rayTracingPipeline.GetPipeline(),
              0, static_cast<uint32_t>(groupCount),
              shaderHandleStorage.size(),
              shaderHandleStorage.data()),
          "get ray tracing shader group handles");
          
    auto pData = static_cast<uint8_t *>(bufferMemory_->Map(0, sbtSize));

    pData += CopyShaderData(pData, rayTracingProperties, rayGenPrograms, rayGenEntrySize_, shaderHandleStorage.data());
    pData += CopyShaderData(pData, rayTracingProperties, missPrograms, missEntrySize_, shaderHandleStorage.data());
    CopyShaderData(pData, rayTracingProperties, hitGroups, hitGroupEntrySize_, shaderHandleStorage.data());

    bufferMemory_->Unmap();
}

estun::ShaderBindingTable::~ShaderBindingTable()
{
    buffer_.reset();
    bufferMemory_.reset();
}
