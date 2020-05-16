#include "renderer/compute_render.h"
#include "renderer/context.h"

estun::ComputeRender::ComputeRender()
{
    ES_CORE_INFO("Creating compute render");
    auto size = static_cast<uint32_t>(ContextLocator::GetSwapChain()->GetImages().size());
    commandBuffers_.reset(new CommandBuffers(CommandPoolLocator::GetComputePool(), size));
    ES_CORE_INFO("* Command buffers done");
}

estun::ComputeRender::~ComputeRender()
{
    pipelines_.clear();
    commandBuffers_.reset();
}

std::shared_ptr<estun::ComputePipeline> estun::ComputeRender::CreatePipeline(
    const std::string computeShaderName,
    const std::shared_ptr<Descriptor> descriptor)
{
    std::shared_ptr<ComputePipeline> pipeline = std::make_shared<ComputePipeline>(computeShaderName, descriptor);
    pipelines_.push_back(pipeline);
    return pipeline;
}

void estun::ComputeRender::Start()
{
    commandBuffers_->Begin(ContextLocator::GetImageIndex());
}

void estun::ComputeRender::End()
{
    commandBuffers_->End(ContextLocator::GetImageIndex());
}

void estun::ComputeRender::Bind(std::shared_ptr<Descriptor> descriptor)
{
    descriptor->Bind(GetCurrCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE);
}

void estun::ComputeRender::Bind(std::shared_ptr<ComputePipeline> pipeline)
{
    pipeline->Bind(GetCurrCommandBuffer());
}

void estun::ComputeRender::Dispath(uint32_t width, uint32_t height)
{
    vkCmdDispatch(GetCurrCommandBuffer(), width / 32, height / 32, 1);
}

VkCommandBuffer &estun::ComputeRender::GetCurrCommandBuffer()
{
    return (*commandBuffers_)[ContextLocator::GetImageIndex()];
}

void estun::ComputeRender::CopyImage(std::shared_ptr<Image> image1, std::shared_ptr<Image> image2)
{
    image1->Barrier(
        GetCurrCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    image2->Barrier(
        GetCurrCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    image1->CopyTo(GetCurrCommandBuffer(), image2);
    image1->Barrier(
        GetCurrCommandBuffer(), VK_IMAGE_LAYOUT_GENERAL,
        VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    image2->Barrier(
        GetCurrCommandBuffer(), VK_IMAGE_LAYOUT_GENERAL,
        VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void estun::ComputeRender::ComputeMemoryBarrier()
{
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = 0;
    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        GetCurrCommandBuffer(),
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}
