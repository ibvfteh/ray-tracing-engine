#include "renderer/graphics_render.h"
#include "renderer/context.h"

estun::GraphicsRender::GraphicsRender(bool toDefault)
    : toDefault_(toDefault)
{
    Create();
}

estun::GraphicsRender::~GraphicsRender()
{
    pipelines_.clear();
    Destroy();
}

void estun::GraphicsRender::Create()
{
    ES_CORE_INFO("Creating graphics render");
    auto msaa = ContextLocator::GetContext()->GetMsaaSamples();
    bool hasMsaa = msaa != VK_SAMPLE_COUNT_1_BIT;
    if (hasMsaa)
    {
        ES_CORE_INFO("* MSAA is on");
    }
    else
    {
        ES_CORE_INFO("* MSAA is off");
    }
    auto size = static_cast<uint32_t>(ContextLocator::GetSwapChain()->GetImages().size());
    commandBuffers_.reset(new CommandBuffers(CommandPoolLocator::GetGraphicsPool(), size));
    ES_CORE_INFO("* Command buffers done");

    colorResources_.reset(new ColorResources(ContextLocator::GetSwapChain()->GetExtent(), VK_SAMPLE_COUNT_1_BIT));
    ES_CORE_INFO("* Color resources done");

    depthResources_.reset(new DepthResources(ContextLocator::GetSwapChain()->GetExtent(), msaa));
    ES_CORE_INFO("* Depth resources done");

    if (hasMsaa)
    {
        colorResolveResources_.reset(new ColorResources(ContextLocator::GetSwapChain()->GetExtent(), msaa));
        ES_CORE_INFO("* Color resolve resources done");
    }

    if (toDefault_)
    {
        renderPass_.reset(new RenderPass(hasMsaa));
    }
    else
    {
        renderPass_.reset(new RenderPass(hasMsaa, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    }

    ES_CORE_INFO("* Render pass done");

    for (int i = 0; i < size; i++)
    {
        std::vector<ImageView *> attachments;

        if (hasMsaa)
        {
            attachments.push_back(colorResolveResources_->GetImageView().get());
        }
        else
        {
            if (toDefault_)
            {
                attachments.push_back(ContextLocator::GetSwapChain()->GetImageViews()[i].get());
            }
            else
            {
                attachments.push_back(colorResources_->GetImageView().get());
            }
        }
        attachments.push_back(depthResources_->GetImageView().get());
        if (hasMsaa)
        {
            if (toDefault_)
            {
                attachments.push_back(ContextLocator::GetSwapChain()->GetImageViews()[i].get());
            }
            else
            {
                attachments.push_back(colorResources_->GetImageView().get());
            }
        }

        framebuffers_.push_back(Framebuffer(attachments, renderPass_));
    }
    ES_CORE_INFO("* Framebuffers done");
}

void estun::GraphicsRender::Destroy()
{
    framebuffers_.clear();
    renderPass_.reset();
    colorResolveResources_.reset();
    depthResources_.reset();
    colorResources_.reset();
    commandBuffers_.reset();
}

void estun::GraphicsRender::Recreate()
{
    Destroy();
    Create();
    for (auto &pipeline : pipelines_)
    {
        pipeline->Recreate(renderPass_);
    }
}

std::shared_ptr<estun::GraphicsPipeline> estun::GraphicsRender::CreatePipeline(
    const std::vector<Shader> shaders,
    const std::shared_ptr<Descriptor> descriptor,
    bool wireFrame)
{
    std::shared_ptr<GraphicsPipeline> pipeline = std::make_shared<GraphicsPipeline>(shaders, renderPass_, descriptor, ContextLocator::GetContext()->GetMsaaSamples(), wireFrame);
    pipelines_.push_back(pipeline);
    return pipeline;
}
/*
std::shared_ptr<estun::RayTracingPipeline> estun::Render::CreateRTPipeline(
    const std::vector<std::string> shaderNames,
    const std::shared_ptr<Descriptor> descriptor)
{
    std::shared_ptr<RayTracingPipeline> rtpipeline = std::make_shared<RayTracingPipeline>(shaderNames, descriptor);
    rtpipelines_.push_back(rtpipeline);
    return rtpipeline;
}
*/
void estun::GraphicsRender::StartDrawInCurrent()
{
    commandBuffers_->Begin(ContextLocator::GetImageIndex());
    renderPass_->Begin(framebuffers_[ContextLocator::GetImageIndex()], GetCurrCommandBuffer());
}

void estun::GraphicsRender::RecordDrawInCurrent()
{
    renderPass_->End(GetCurrCommandBuffer());
    commandBuffers_->End(ContextLocator::GetImageIndex());
}

void estun::GraphicsRender::Bind(std::shared_ptr<Descriptor> descriptor)
{
    descriptor->Bind(GetCurrCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void estun::GraphicsRender::Bind(std::shared_ptr<GraphicsPipeline> pipeline)
{
    pipeline->Bind(GetCurrCommandBuffer());
}

void estun::GraphicsRender::Bind(std::shared_ptr<VertexBuffer> vertexBuffer)
{
    vertexBuffer->Bind(GetCurrCommandBuffer());
}

void estun::GraphicsRender::Bind(std::shared_ptr<IndexBuffer> indexBuffer)
{
    indexBuffer->Bind(GetCurrCommandBuffer());
}

VkCommandBuffer &estun::GraphicsRender::GetCurrCommandBuffer()
{
    return (*commandBuffers_)[ContextLocator::GetImageIndex()];
}

void estun::GraphicsRender::DrawIndexed(uint32_t indexesSize, uint32_t indexOffset, uint32_t vertexOffset)
{
    vkCmdDrawIndexed(GetCurrCommandBuffer(), indexesSize, 1, indexOffset, vertexOffset, 0);
}
