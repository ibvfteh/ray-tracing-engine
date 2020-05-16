#include "renderer/material/compute_pipeline.h"
#include "renderer/context/device.h"
#include "renderer/material/shader_module.h"
#include "renderer/context.h"
#include "renderer/material/descriptor.h"
#include "renderer/material/pipeline_layout.h"

estun::ComputePipeline::ComputePipeline(
    const std::string computeShaderName,
    std::shared_ptr<Descriptor> descriptor)
    : descriptor_(descriptor)
{
    computeShaderModule_ = std::make_unique<ShaderModule>(computeShaderName);

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = descriptor->GetPipelineLayout().GetPipelineLayout();
    pipelineInfo.flags = 0;
    pipelineInfo.stage = computeShaderModule_->CreateShaderStage(VK_SHADER_STAGE_COMPUTE_BIT);
    
    VK_CHECK_RESULT(vkCreateComputePipelines(DeviceLocator::GetLogicalDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline_), "Failed to create compute pipeline");
}

estun::ComputePipeline::~ComputePipeline()
{
    if (pipeline_ != nullptr)
    {
        vkDestroyPipeline(DeviceLocator::GetLogicalDevice(), pipeline_, nullptr);
        pipeline_ = nullptr;
    }
}

void estun::ComputePipeline::Bind(VkCommandBuffer &commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
}
