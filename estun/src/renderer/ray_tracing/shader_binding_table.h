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
        ShaderBindingTable(const ShaderBindingTable &) = delete;
        ShaderBindingTable(ShaderBindingTable &&) = delete;
        ShaderBindingTable &operator=(const ShaderBindingTable &) = delete;
        ShaderBindingTable &operator=(ShaderBindingTable &&) = delete;

        ShaderBindingTable(
            const std::shared_ptr<RayTracingPipeline> rayTracingPipeline,
            std::vector<uint32_t> programs);

        ~ShaderBindingTable();

        const Buffer &GetBuffer() const { return *buffer_; }

        size_t GetRayGenOffset() const { return rayGenOffset_; }
        size_t GetMissOffset() const { return missOffset_; }
        size_t GetHitGroupOffset() const { return hitGroupOffset_; }

        size_t GetRayGenEntrySize() const { return rayGenEntrySize_; }
        size_t GetMissEntrySize() const { return missEntrySize_; }
        size_t GetHitGroupEntrySize() const { return hitGroupEntrySize_; }

        
        size_t GetSize() const { return sbtSize_; }

    private:
        size_t rayGenEntrySize_;
        size_t missEntrySize_;
        size_t hitGroupEntrySize_;

        size_t rayGenOffset_;
        size_t missOffset_;
        size_t hitGroupOffset_;

        size_t sbtSize_;

        std::unique_ptr<Buffer> buffer_;
        std::unique_ptr<DeviceMemory> bufferMemory_;
    };

} // namespace estun
