#include "renderer/ray_tracing/ray_tracing_pipeline.h"
#include "renderer/ray_tracing/top_level_acceleration_structure.h"
#include "renderer/material/pipeline_layout.h"
#include "renderer/material/shader_module.h"
#include "renderer/context.h"
#include "renderer/context/image_view.h"
#include "renderer/buffers/uniform_buffer.h"
#include "renderer/material/descriptor.h"
#include "renderer/material/descriptor_binding.h"
#include "renderer/context/dynamic_functions.h"

estun::RayTracingPipeline::RayTracingPipeline(
    const std::vector<std::string> shaders,
    const Descriptor &descriptor)
{
    // Load shaders.
    const ShaderModule rayGenShader(shaders[0]);
    const ShaderModule missShader(shaders[1]);
    const ShaderModule closestHitShader(shaders[2]);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
        {
            rayGenShader.CreateShaderStage(VK_SHADER_STAGE_RAYGEN_BIT_NV),
            missShader.CreateShaderStage(VK_SHADER_STAGE_MISS_BIT_NV),
            closestHitShader.CreateShaderStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV)};

    // Shader groups
    VkRayTracingShaderGroupCreateInfoNV rayGenGroupInfo = {};
    rayGenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
    rayGenGroupInfo.pNext = nullptr;
    rayGenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    rayGenGroupInfo.generalShader = 0;
    rayGenGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
    rayGenGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
    rayGenGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
    rayGenIndex_ = 0;

    VkRayTracingShaderGroupCreateInfoNV missGroupInfo = {};
    missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
    missGroupInfo.pNext = nullptr;
    missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    missGroupInfo.generalShader = 1;
    missGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
    missGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
    missGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
    missIndex_ = 1;

    VkRayTracingShaderGroupCreateInfoNV triangleHitGroupInfo = {};
    triangleHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
    triangleHitGroupInfo.pNext = nullptr;
    triangleHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
    triangleHitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
    triangleHitGroupInfo.closestHitShader = 2;
    triangleHitGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
    triangleHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
    triangleHitGroupIndex_ = 2;

    std::vector<VkRayTracingShaderGroupCreateInfoNV> groups =
        {
            rayGenGroupInfo,
            missGroupInfo,
            triangleHitGroupInfo,
        };

    // Create graphic pipeline
    VkRayTracingPipelineCreateInfoNV pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(groups.size());
    pipelineInfo.pGroups = groups.data();
    pipelineInfo.maxRecursionDepth = 1;
    pipelineInfo.layout = descriptor.GetPipelineLayout().GetPipelineLayout();
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = 0;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkCreateRayTracingPipelinesNV(DeviceLocator::GetLogicalDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline_), "create ray tracing pipeline");
}

estun::RayTracingPipeline::~RayTracingPipeline()
{
    if (pipeline_ != nullptr)
    {
        vkDestroyPipeline(DeviceLocator::GetLogicalDevice(), pipeline_, nullptr);
        pipeline_ = nullptr;
    }
}

void estun::RayTracingPipeline::Bind(VkCommandBuffer &commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, pipeline_);
}

VkPipeline estun::RayTracingPipeline::GetPipeline() const
{
    return pipeline_;
}
