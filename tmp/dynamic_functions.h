#pragma once

#include "renderer/common.h"

namespace estun
{

    class DynamicFunctions
    {
    public:
        DynamicFunctions(const DynamicFunctions &) = delete;
        DynamicFunctions(DynamicFunctions &&) = delete;

        DynamicFunctions &operator=(const DynamicFunctions &) = delete;
        DynamicFunctions &operator=(DynamicFunctions &&) = delete;

        explicit DynamicFunctions();
        ~DynamicFunctions();

        const std::function<VkResult(
            VkDevice device,
            const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
            const VkAllocationCallbacks *pAllocator,
            VkAccelerationStructureKHR *pAccelerationStructure)>
            vkCreateAccelerationStructureKHR;

        const std::function<void(
            VkDevice device,
            VkAccelerationStructureKHR accelerationStructure,
            const VkAllocationCallbacks *pAllocator)>
            vkDestroyAccelerationStructureKHR;

        const std::function<void(
            VkDevice device,
            const VkAccelerationStructureMemoryRequirementsInfoKHR *pInfo,
            VkMemoryRequirements2 *pMemoryRequirements)>
            vkGetAccelerationStructureMemoryRequirementsKHR;

        const std::function<VkResult(
            VkDevice device,
            uint32_t bindInfoCount,
            const VkBindAccelerationStructureMemoryInfoKHR *pBindInfos)>
            vkBindAccelerationStructureMemoryKHR;

        const std::function<void(
            VkCommandBuffer commandBuffer,
            uint32_t infoCount,
            const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
            const VkAccelerationStructureBuildOffsetInfoKHR *const *ppOffsetInfos)>
            vkCmdBuildAccelerationStructureKHR;

        const std::function<VkDeviceAddress(
            VkDevice device,
            const VkAccelerationStructureDeviceAddressInfoKHR *pInfo)>
            vkGetAccelerationStructureDeviceAddressKHR;

        const std::function<VkResult(
            VkDevice device,
            VkPipelineCache pipelineCache,
            uint32_t createInfoCount,
            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
            const VkAllocationCallbacks *pAllocator,
            VkPipeline *pPipelines)>
            vkCreateRayTracingPipelinesKHR;

        const std::function<VkResult(
            VkDevice device,
            VkPipeline pipeline,
            uint32_t firstGroup,
            uint32_t groupCount,
            size_t dataSize,
            void *pData)>
            vkGetRayTracingShaderGroupHandlesKHR;

        const std::function<void(
            VkCommandBuffer commandBuffer,
            const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
            const VkStridedBufferRegionKHR *pMissShaderBindingTable,
            const VkStridedBufferRegionKHR *pHitShaderBindingTable,
            const VkStridedBufferRegionKHR *pCallableShaderBindingTable,
            uint32_t width,
            uint32_t height,
            uint32_t depth)>
            vkCmdTraceRaysKHR;
    };

    class FunctionsLocator
    {
    public:
        static DynamicFunctions &GetFunctions() { return *funcs_; };

        static void Provide(DynamicFunctions *funcs) { funcs_ = funcs; };

    private:
        static DynamicFunctions *funcs_;
    };

} // namespace estun