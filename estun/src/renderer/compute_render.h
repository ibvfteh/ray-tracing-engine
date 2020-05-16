#pragma once

#include "renderer/common.h"

#include "core/core.h"

#include "renderer/context/command_pool.h"
#include "renderer/context/command_buffers.h"
#include "renderer/context/resources.h"
#include "renderer/context/render_pass.h"
#include "renderer/context/framebuffer.h"
#include "renderer/material/compute_pipeline.h"
#include "renderer/material/descriptor.h"
#include "renderer/material/pipeline_layout.h"
#include "renderer/buffers/storage_buffer.h"
#include "renderer/model.h"
#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/shader_bilding_table.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/base_image.h"
#include "renderer/context/image_view.h"

#include "renderer/context/image.h"

namespace estun
{

    class ComputeRender
    {
    public:
        ComputeRender(const ComputeRender &) = delete;
        ComputeRender(ComputeRender &&) = delete;

        ComputeRender &operator=(const ComputeRender &) = delete;
        ComputeRender &operator=(ComputeRender &&) = delete;

        ComputeRender();
        ~ComputeRender();

        std::shared_ptr<ComputePipeline> CreatePipeline(
            const std::string computeShaderName,
            const std::shared_ptr<Descriptor> descriptor);

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
        void Bind(std::shared_ptr<ComputePipeline> pipeline);

        void CopyImage(std::shared_ptr<Image> image1, std::shared_ptr<Image> image2);
        void ComputeMemoryBarrier();

        void Start();
        void End();
        void Dispath(uint32_t width, uint32_t height);

        VkCommandBuffer &GetCurrCommandBuffer();

    private:
        std::unique_ptr<CommandBuffers> commandBuffers_;

        std::vector<std::shared_ptr<Image>> colorImages_;

        std::vector<std::shared_ptr<ComputePipeline>> pipelines_;

        VkCommandBuffer currCommandBuffer_;
    };

} // namespace estun