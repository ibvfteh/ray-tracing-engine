#pragma once

#include "renderer/common.h"

namespace estun
{
    class Descriptor;
    class ShaderModule;

    class ComputePipeline
    {
    public:
        ComputePipeline(const ComputePipeline &) = delete;
        ComputePipeline(ComputePipeline &&) = delete;

        ComputePipeline &operator=(const ComputePipeline &) = delete;
        ComputePipeline &operator=(ComputePipeline &&) = delete;

        ComputePipeline(
            const std::string computeShaderName,
            std::shared_ptr<Descriptor> descriptor);
        ~ComputePipeline();

        void Bind(VkCommandBuffer &commandBuffer);

        VkDescriptorSet GetDescriptorSet(uint32_t index) const;

    private:
        VkPipeline pipeline_;

        std::shared_ptr<Descriptor> descriptor_;

        std::unique_ptr<ShaderModule> computeShaderModule_;
    };

} // namespace estun