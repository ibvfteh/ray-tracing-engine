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
    const std::vector<std::vector<Shader>> shaderGroups,
    const std::shared_ptr<Descriptor> descriptor)
{
    // Load shaders.

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<std::shared_ptr<ShaderModule>> shaderModules;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
    int groupIndex = 0;
    int index = 0;
    for (auto &group : shaderGroups)
    {
        VkRayTracingShaderGroupCreateInfoKHR groupInfo = {};
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        groupInfo.pNext = nullptr;
        groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        int groupInfoType = -1;
        groupInfo.generalShader = VK_SHADER_UNUSED_KHR;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
        VkShaderStageFlagBits groupShaderStage;
        for (auto &shader : group)
        {
            shaderModules.push_back(std::make_shared<ShaderModule>(shader.name));
            shaderStages.push_back(shaderModules.back()->CreateShaderStage(shader.bits));
            switch (shader.bits)
            {
            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                if ((groupInfoType != -1) && (groupInfoType != VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR ))
                {
                    ES_CORE_ASSERT("Inctersection shader can only be in procedural hit group");
                }
                if (groupInfo.intersectionShader != VK_SHADER_UNUSED_KHR)
                {
                    ES_CORE_ASSERT("Two equal shaders in the same group");
                } 
                groupInfoType = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
                groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
                groupInfo.intersectionShader = index;
                break;

            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                if (groupInfoType == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR  )
                {
                    ES_CORE_ASSERT("Closest hit shader can only be in hit group");
                }
                if (groupInfo.closestHitShader != VK_SHADER_UNUSED_KHR)
                {
                    ES_CORE_ASSERT("Two equal shaders in the same group");
                }
                if(groupInfoType == -1)
                {
                    groupInfoType = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                    groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                }
                groupInfo.closestHitShader = index;
                break;

            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                if (groupInfoType == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR  )
                {
                    ES_CORE_ASSERT("Any hit shader can only be in hit group");
                }
                if (groupInfo.anyHitShader != VK_SHADER_UNUSED_KHR)
                {
                    ES_CORE_ASSERT("Two equal shaders in the same group");
                }
                if(groupInfoType == -1)
                {
                    groupInfoType = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                    groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                }
                groupInfo.anyHitShader = index;
                break;

            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            case VK_SHADER_STAGE_MISS_BIT_KHR:
            case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
                if (groupInfoType != -1)
                {
                    ES_CORE_ASSERT("Ray gen shader can only be in general group");
                }
                if (groupInfo.generalShader != VK_SHADER_UNUSED_KHR)
                {
                    ES_CORE_ASSERT("Two equal shaders in the same group");
                }
                groupInfoType = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                groupShaderStage = shader.bits;
                groupInfo.generalShader = index;
                break;

            default:
                ES_CORE_ASSERT("Unrecognized shader type");
                break;
            }
            index++;
        }
        if (groupInfoType == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR)
        {
            switch(groupShaderStage)
            {
            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                rayGenGroups.push_back(groupIndex);
                break;

            case VK_SHADER_STAGE_MISS_BIT_KHR:
                missGroups.push_back(groupIndex);
                break;

            case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
                callGroups.push_back(groupIndex);
                break;
            }
        }
        else
        {
            hitGroups.push_back(groupIndex);
        }
        groups.push_back(groupInfo);
        groupIndex++;
    }

    // Create graphic pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(groups.size());
    pipelineInfo.pGroups = groups.data();
    pipelineInfo.maxRecursionDepth = 1;
    pipelineInfo.libraries = {};
    pipelineInfo.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
    pipelineInfo.libraries.pNext = nullptr;
    pipelineInfo.libraries.libraryCount = 0;
    pipelineInfo.libraries.pLibraries = nullptr;
    pipelineInfo.pLibraryInterface = nullptr;
    pipelineInfo.layout = descriptor->GetPipelineLayout().GetPipelineLayout();
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
