#include "renderer/context/dynamic_functions.h"
#include "renderer/context/device.h"
#include <string>

namespace
{

    template <class Func>
    Func GetProcedure(const char *const name)
    {
        const auto func = reinterpret_cast<Func>(vkGetDeviceProcAddr(estun::DeviceLocator::GetLogicalDevice(), name));
        if (func == nullptr)
        {
            ES_CORE_ASSERT(std::string("failed to get address of '") + name + std::string("'"));
        }

        return func;
    }

} // namespace

estun::DynamicFunctions::DynamicFunctions()
    : vkCreateAccelerationStructureKHR(GetProcedure<PFN_vkCreateAccelerationStructureKHR>("vkCreateAccelerationStructureKHR")),
      vkDestroyAccelerationStructureKHR(GetProcedure<PFN_vkDestroyAccelerationStructureKHR>("vkDestroyAccelerationStructureKHR")),
      vkGetAccelerationStructureMemoryRequirementsKHR(GetProcedure<PFN_vkGetAccelerationStructureMemoryRequirementsKHR>("vkGetAccelerationStructureMemoryRequirementsKHR")),
      vkBindAccelerationStructureMemoryKHR(GetProcedure<PFN_vkBindAccelerationStructureMemoryKHR>("vkBindAccelerationStructureMemoryKHR")),
      vkGetAccelerationStructureDeviceAddressKHR(GetProcedure<PFN_vkGetAccelerationStructureDeviceAddressKHR>("vkGetAccelerationStructureDeviceAddressKHR")),
      vkCmdBuildAccelerationStructureKHR(GetProcedure<PFN_vkCmdBuildAccelerationStructureKHR>("vkCmdBuildAccelerationStructureKHR")),
      vkCreateRayTracingPipelinesKHR(GetProcedure<PFN_vkCreateRayTracingPipelinesKHR>("vkCreateRayTracingPipelinesKHR")),
      vkGetRayTracingShaderGroupHandlesKHR(GetProcedure<PFN_vkGetRayTracingShaderGroupHandlesKHR>("vkGetRayTracingShaderGroupHandlesKHR")),
      vkCmdTraceRaysKHR(GetProcedure<PFN_vkCmdTraceRaysKHR>("vkCmdTraceRaysKHR"))
{
}

estun::DynamicFunctions::~DynamicFunctions()
{
}

estun::DynamicFunctions *estun::FunctionsLocator::funcs_ = nullptr;