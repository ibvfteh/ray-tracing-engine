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
#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/shader_binding_table.h"
#include "renderer/buffers/buffer.h"
#include "renderer/context/base_image.h"
#include "renderer/context/image_view.h"

#include "renderer/context/image.h"

namespace estun
{

    class RayTracingRender
    {
    public:
        RayTracingRender(const RayTracingRender &) = delete;
        RayTracingRender(RayTracingRender &&) = delete;

        RayTracingRender &operator=(const RayTracingRender &) = delete;
        RayTracingRender &operator=(RayTracingRender &&) = delete;

        RayTracingRender();
        ~RayTracingRender();

        std::shared_ptr<RayTracingPipeline> CreatePipeline(
            const std::vector<std::vector<Shader>> shaderGroups,
            const std::shared_ptr<Descriptor> descriptor);

        void BeginBuffer();
        void EndBuffer();

        void Bind(std::shared_ptr<Descriptor> descriptor);
        void Bind(std::shared_ptr<RayTracingPipeline> pipeline);

        void TraceRays(std::shared_ptr<ShaderBindingTable> sbtable, uint32_t width, uint32_t height);

        void CopyImage(std::shared_ptr<Image> image1, std::shared_ptr<Image> image2);

        VkCommandBuffer &GetCurrCommandBuffer();
        std::vector<std::shared_ptr<RayTracingPipeline>> &GetPipelines() { return pipelines_; }

    private:
        std::unique_ptr<CommandBuffers> commandBuffers_;

        std::vector<std::shared_ptr<RayTracingPipeline>> pipelines_;

        VkCommandBuffer currCommandBuffer_;
    };

} // namespace estun