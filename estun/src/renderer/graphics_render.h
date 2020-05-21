#pragma once

#include "renderer/common.h"

#include "core/core.h"

#include "renderer/context/command_pool.h"
#include "renderer/context/command_buffers.h"
#include "renderer/context/resources.h"
#include "renderer/context/render_pass.h"
#include "renderer/context/framebuffer.h"
#include "renderer/material/graphics_pipeline.h"
#include "renderer/material/descriptor.h"
#include "renderer/material/pipeline_layout.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/model.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/base_image.h"
#include "renderer/context/image_view.h"

#include "renderer/context/image.h"

namespace estun
{

    class GraphicsRender
    {
    public:
        GraphicsRender(const GraphicsRender &) = delete;
        GraphicsRender(GraphicsRender &&) = delete;

        GraphicsRender &operator=(const GraphicsRender &) = delete;
        GraphicsRender &operator=(GraphicsRender &&) = delete;

        GraphicsRender(bool toDefault = true);
        ~GraphicsRender();

        void Create();
        void Destroy();
        void Recreate();
            
        std::shared_ptr<GraphicsPipeline> CreatePipeline(
            const std::vector<Shader> shaders,  
            const std::shared_ptr<Descriptor> descriptor,
            bool wireFrame = false);

        void StartDrawInCurrent();
        void RecordDrawInCurrent();

        template <class T>
        void Bind(PushConstant<T> &pushConstant, std::shared_ptr<Descriptor> descriptor)
        {
            vkCmdPushConstants(
                GetCurrCommandBuffer(),
                descriptor->GetPipelineLayout().GetPipelineLayout(),
                pushConstant.stageFlags_, 0,
                sizeof(T), pushConstant.GetConst());
        }
        void Bind(std::shared_ptr<Descriptor> descriptor);
        void Bind(std::shared_ptr<GraphicsPipeline> pipeline);
        void Bind(std::shared_ptr<VertexBuffer> vertexBuffer);
        void Bind(std::shared_ptr<IndexBuffer> indexBuffer);
        void DrawIndexed(uint32_t indexesSize, uint32_t indexOffset, uint32_t vertexOffset);

        VkCommandBuffer &GetCurrCommandBuffer();
        RenderPass &GetRenderPass() { return *renderPass_; };
        std::shared_ptr<ColorResources> GetColorResources() { return colorResources_; };
        std::shared_ptr<DepthResources> GetDepthResources() { return depthResources_; };
        std::vector<std::shared_ptr<GraphicsPipeline>> &GetPipelines() { return pipelines_; }

    private:
        bool toDefault_;
        std::unique_ptr<CommandBuffers> commandBuffers_;
        std::shared_ptr<ColorResources> colorResources_;
        std::shared_ptr<DepthResources> depthResources_;
        std::unique_ptr<ColorResources> colorResolveResources_;
        std::unique_ptr<RenderPass> renderPass_;
        std::vector<Framebuffer> framebuffers_;

        std::vector<std::shared_ptr<GraphicsPipeline>> pipelines_;

        VkSampleCountFlagBits msaa_;

        VkCommandBuffer currCommandBuffer_;
    };

} // namespace estun