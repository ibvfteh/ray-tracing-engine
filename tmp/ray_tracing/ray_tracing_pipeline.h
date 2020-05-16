#pragma once

#include "renderer/common.h"
#include "renderer/context/utils.h"

namespace estun
{

class DeviceProcedures;
class TopLevelAccelerationStructure;
class ImageView;
class Descriptor;
class DescriptorBinding;

class RayTracingPipeline
{
public:
    RayTracingPipeline(const RayTracingPipeline &) = delete;
    RayTracingPipeline(RayTracingPipeline &&) = delete;
    RayTracingPipeline &operator=(const RayTracingPipeline &) = delete;
    RayTracingPipeline &operator=(RayTracingPipeline &&) = delete;

    RayTracingPipeline(
        const std::vector<std::string> shaders,
        const Descriptor &descriptor);
    ~RayTracingPipeline();

    void Bind(VkCommandBuffer &commandBuffer);

    uint32_t RayGenShaderIndex() const { return rayGenIndex_; }
    uint32_t MissShaderIndex() const { return missIndex_; }
    uint32_t TriangleHitGroupIndex() const { return triangleHitGroupIndex_; }

    VkDescriptorSet DescriptorSet(uint32_t index) const;

    VkPipeline GetPipeline() const;

private:
    VkPipeline pipeline_;

    uint32_t rayGenIndex_;
    uint32_t missIndex_;
    uint32_t triangleHitGroupIndex_;
};

} // namespace estun
