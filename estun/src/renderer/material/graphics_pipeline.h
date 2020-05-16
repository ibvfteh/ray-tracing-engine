#pragma once

#include "renderer/common.h"
#include "renderer/material/shader.h"

namespace estun
{

    class RenderPass;
    class Descriptor;
    class ShaderModule;

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(const GraphicsPipeline &) = delete;
        GraphicsPipeline(GraphicsPipeline &&) = delete;

        GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
        GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

        GraphicsPipeline(
            const std::vector<Shader> shaders,
            std::unique_ptr<RenderPass> &renderPass,
            std::shared_ptr<Descriptor> descriptor,
            VkSampleCountFlagBits msaa,
            bool isWireFrame);

        ~GraphicsPipeline();

        void Create(std::unique_ptr<RenderPass> &renderPass);
        void Destroy();
        void Recreate(std::unique_ptr<RenderPass> &renderPass);

        void Bind(VkCommandBuffer &commandBuffer);

        VkDescriptorSet GetDescriptorSet(uint32_t index) const;
        bool IsWireFrame() const { return isWireFrame_; }

    private:
        const bool isWireFrame_;
        VkSampleCountFlagBits msaa_;

        VkPipeline pipeline_;

        std::shared_ptr<Descriptor> descriptor_;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;
        std::vector<std::shared_ptr<ShaderModule>> shaderModules_;
    };

} // namespace estun