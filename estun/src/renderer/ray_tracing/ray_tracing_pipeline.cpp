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
    const std::vector<Shader> shaders,
    const Descriptor &descriptor)
{
    // Load shaders.

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<std::shared_ptr<ShaderModule>> shaderModules;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    int index = 0;
    uint32_t closestHitShader = VK_SHADER_UNUSED_KHR;
    uint32_t anyHitShader = VK_SHADER_UNUSED_KHR;
    for (auto &s : shaders)
    {
        shaderModules.push_back(std::make_shared<ShaderModule>(s.name));
        shaderStages.push_back(shaderModules.back()->CreateShaderStage(s.bits));
        switch (s.bits)
        {
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            ES_CORE_ASSERT("Intersection shaders not handeled");
            break;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            closestHitShader = index;
            break;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            anyHitShader = index;
            break;
        default:
            VkRayTracingShaderGroupCreateInfoKHR groupInfo = {};
            groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            groupInfo.pNext = nullptr;
            groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            groupInfo.generalShader = index;
            groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(groupInfo);
            break;
        }
        index++;
    }
    if (!(closestHitShader == VK_SHADER_UNUSED_KHR) && !(anyHitShader == VK_SHADER_UNUSED_KHR))
    {
        VkRayTracingShaderGroupCreateInfoKHR groupInfo = {};
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        groupInfo.pNext = nullptr;
        groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        groupInfo.generalShader = index;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(groupInfo);
    }

    // Create graphic pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();
    pipelineInfo.maxRecursionDepth = 1;
    pipelineInfo.layout = descriptor.GetPipelineLayout().GetPipelineLayout();
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = 0;

    VK_CHECK_RESULT(FunctionsLocator::GetFunctions().vkCreateRayTracingPipelinesKHR(DeviceLocator::GetLogicalDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline_), "create ray tracing pipeline");
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_);
}

VkPipeline estun::RayTracingPipeline::GetPipeline() const
{
    return pipeline_;
}
