#pragma once

#include "renderer/common.h"
#include "renderer/context/utils.h"
#include "renderer/material/shader.h"

namespace estun
{

    class DeviceProcedures;
    class TopLevelAccelerationStructure;
    class ImageView;
    class Descriptor;
    class DescriptorBinding;

    enum ShaderGroups
    {
        RayGen = 0,
        Miss = 1,
        Hit = 2,
        Call = 3,
    };

    class RayTracingPipeline
    {
    public:
        RayTracingPipeline(const RayTracingPipeline &) = delete;
        RayTracingPipeline(RayTracingPipeline &&) = delete;
        RayTracingPipeline &operator=(const RayTracingPipeline &) = delete;
        RayTracingPipeline &operator=(RayTracingPipeline &&) = delete;

        RayTracingPipeline(
            const std::vector<std::vector<Shader>> shaderGroups,
            const std::shared_ptr<Descriptor> descriptor);
        ~RayTracingPipeline();

        void Bind(VkCommandBuffer &commandBuffer);
        
        uint32_t GetGroupCount() const { return !rayGenGroups.empty() + !missGroups.empty() + !hitGroups.empty() + !callGroups.empty(); }

        VkDescriptorSet DescriptorSet(uint32_t index) const;

        VkPipeline GetPipeline() const;

    private:
        VkPipeline pipeline_;

        std::vector<uint32_t> rayGenGroups = {};
        std::vector<uint32_t> missGroups = {};
        std::vector<uint32_t> hitGroups = {};
        std::vector<uint32_t> callGroups = {};
    };

} // namespace estun
