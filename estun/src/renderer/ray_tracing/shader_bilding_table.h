#pragma once

#include "renderer/common.h"

namespace estun
{

class RayTracingPipeline;
class RayTracingProperties;
class Buffer;
class DeviceMemory;

class ShaderBindingTable
{
public:
    struct Entry
    {
        uint32_t GroupIndex;
        std::vector<unsigned char> InlineData;
    };

    ShaderBindingTable(const ShaderBindingTable &) = delete;
    ShaderBindingTable(ShaderBindingTable &&) = delete;
    ShaderBindingTable &operator=(const ShaderBindingTable &) = delete;
    ShaderBindingTable &operator=(ShaderBindingTable &&) = delete;

    ShaderBindingTable(
        const RayTracingPipeline &rayTracingPipeline,
        const RayTracingProperties &rayTracingProperties,
        const std::vector<Entry> &rayGenPrograms,
        const std::vector<Entry> &missPrograms,
        const std::vector<Entry> &hitGroups);

    ~ShaderBindingTable();

    const Buffer &GetBuffer() const { return *buffer_; }

    size_t RayGenOffset() const { return rayGenOffset_; }
    size_t MissOffset() const { return missOffset_; }
    size_t HitGroupOffset() const { return hitGroupOffset_; }

    size_t RayGenEntrySize() const { return rayGenEntrySize_; }
    size_t MissEntrySize() const { return missEntrySize_; }
    size_t HitGroupEntrySize() const { return hitGroupEntrySize_; }

private:
    const size_t rayGenEntrySize_;
    const size_t missEntrySize_;
    const size_t hitGroupEntrySize_;

    const size_t rayGenOffset_;
    const size_t missOffset_;
    const size_t hitGroupOffset_;

    std::unique_ptr<Buffer> buffer_;
    std::unique_ptr<DeviceMemory> bufferMemory_;
};

} // namespace estun
