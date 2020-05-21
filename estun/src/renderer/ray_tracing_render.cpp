#include "renderer/ray_tracing_render.h"
#include "renderer/context.h"

estun::RayTracingRender::RayTracingRender()
{
    ES_CORE_INFO("Creating compute render");
    auto size = static_cast<uint32_t>(ContextLocator::GetSwapChain()->GetImages().size());
    commandBuffers_.reset(new CommandBuffers(CommandPoolLocator::GetComputePool(), size));
    ES_CORE_INFO("* Command buffers done");
}

estun::RayTracingRender::~RayTracingRender()
{
    pipelines_.clear();
    commandBuffers_.reset();
}

std::shared_ptr<estun::RayTracingPipeline> estun::RayTracingRender::CreatePipeline(
    const std::vector<Shader> shaders,
    const std::shared_ptr<Descriptor> descriptor)
{
    std::shared_ptr<RayTracingPipeline> pipeline = std::make_shared<RayTracingPipeline>(shaders, descriptor);
    pipelines_.push_back(pipeline);
    return pipeline;
}

void estun::RayTracingRender::BeginBuffer()
{
    commandBuffers_->Begin(ContextLocator::GetImageIndex());
}

void estun::RayTracingRender::EndBuffer()
{
    commandBuffers_->End(ContextLocator::GetImageIndex());
}

void estun::RayTracingRender::Bind(std::shared_ptr<Descriptor> descriptor)
{
    descriptor->Bind(GetCurrCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE);
}

void estun::RayTracingRender::Bind(std::shared_ptr<RayTracingPipeline> pipeline)
{
    pipeline->Bind(GetCurrCommandBuffer());
}

void estun::RayTracingRender::TraceRays(std::shared_ptr<ShaderBindingTable> sbtable, uint32_t width, uint32_t height)
{
    const VkStridedBufferRegionKHR raygenShaderBindingTable = {sbtable->GetBuffer().GetBuffer(), sbtable->GetRayGenOffset(), sbtable->GetRayGenEntrySize(), sbtable->GetSize()};
    const VkStridedBufferRegionKHR missShaderBindingTable = {sbtable->GetBuffer().GetBuffer(), sbtable->GetMissOffset(), sbtable->GetMissEntrySize(), sbtable->GetSize()};
    const VkStridedBufferRegionKHR hitShaderBindingTable = {sbtable->GetBuffer().GetBuffer(), sbtable->GetHitGroupOffset(), sbtable->GetHitGroupEntrySize(), sbtable->GetSize()};
    const VkStridedBufferRegionKHR callableShaderBindingTable = {};

    FunctionsLocator::GetFunctions().vkCmdTraceRaysKHR(
        GetCurrCommandBuffer(),
        &raygenShaderBindingTable,
        &missShaderBindingTable,
        &hitShaderBindingTable,
        &callableShaderBindingTable,
        width, height, 1);
}

void estun::RayTracingRender::CopyImage(std::shared_ptr<Image> image1, std::shared_ptr<Image> image2)
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

VkCommandBuffer &estun::RayTracingRender::GetCurrCommandBuffer()
{
    return (*commandBuffers_)[ContextLocator::GetImageIndex()];
}

/*
void estun::GraphicsRender::CreateRayTracingOutputImage()
{
    const auto extent = ContextLocator::GetSwapChain()->GetExtent();
    const auto format = ContextLocator::GetSwapChain()->GetFormat();
    const auto tiling = VK_IMAGE_TILING_OPTIMAL;

    accumulationImage_.reset(
        new Image(
            extent.width, extent.height,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT));

    outputImage_.reset(
        new Image(
            extent.width, extent.height,
            VK_SAMPLE_COUNT_1_BIT, format, tiling,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT));
}

void estun::GraphicsRender::DeleteRayTracingOutputImage()
{
    accumulationImage_.reset();
    outputImage_.reset();
}
void estun::Context::CreateRayTracingOutputImage()
{
    const auto extent = swapChain_->GetExtent();
    const auto format = swapChain_->GetFormat();
    const auto tiling = VK_IMAGE_TILING_OPTIMAL;

    accumulationImage_.reset(new Image(
        extent.width, extent.height,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT));

    outputImage_.reset(new Image(
        extent.width, extent.height,
        format,
        tiling,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT));
}

void estun::Context::DeleteRayTracingOutputImage()
{
    accumulationImage_.reset();
    outputImage_.reset();
}

void estun::Context::StartRayTracing()
{
    const auto extent = swapChain_->GetExtent();

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    ImageMemoryBarrier::Insert(currCommandBuffer_, accumulationImage_->GetImage().GetImage(), subresourceRange, 0,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    ImageMemoryBarrier::Insert(currCommandBuffer_, outputImage_->GetImage().GetImage(), subresourceRange, 0,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void estun::Context::EndRayTracing(ShaderBindingTable &shaderBindingTable)
{
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    vkCmdTraceRaysNV(currCommandBuffer_,
                     shaderBindingTable.GetBuffer().GetBuffer(), shaderBindingTable.RayGenOffset(),
                     shaderBindingTable.GetBuffer().GetBuffer(), shaderBindingTable.MissOffset(), shaderBindingTable.MissEntrySize(),
                     shaderBindingTable.GetBuffer().GetBuffer(), shaderBindingTable.HitGroupOffset(), shaderBindingTable.HitGroupEntrySize(),
                     nullptr, 0, 0,
                     swapChain_->GetExtent().width, swapChain_->GetExtent().height, 1);

    ImageMemoryBarrier::Insert(currCommandBuffer_, outputImage_->GetImage().GetImage(), subresourceRange,
                               VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    ImageMemoryBarrier::Insert(currCommandBuffer_, swapChain_->GetImages()[imageIndex_], subresourceRange, 0,
                               VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copyRegion;
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {swapChain_->GetExtent().width, swapChain_->GetExtent().height, 1};

    vkCmdCopyImage(currCommandBuffer_,
                   outputImage_->GetImage().GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   swapChain_->GetImages()[imageIndex_], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &copyRegion);

    ImageMemoryBarrier::Insert(currCommandBuffer_, swapChain_->GetImages()[imageIndex_], subresourceRange, VK_ACCESS_TRANSFER_WRITE_BIT,
                               0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}
*/
