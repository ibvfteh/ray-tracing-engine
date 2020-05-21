#pragma once

#include "renderer/common.h"

namespace estun
{

    class RayTracingProperties
    {
    public:
        explicit RayTracingProperties();

        uint32_t MaxDescriptorSetAccelerationStructures() const { return rayTracingProperties_.maxDescriptorSetAccelerationStructures; }
        uint64_t MaxGeometryCount() const { return rayTracingProperties_.maxGeometryCount; }
        uint64_t MaxInstanceCount() const { return rayTracingProperties_.maxInstanceCount; }
        uint32_t MaxRecursionDepth() const { return rayTracingProperties_.maxRecursionDepth; }
        uint32_t MaxShaderGroupStride() const { return rayTracingProperties_.maxShaderGroupStride; }
        //uint64_t MaxTriangleCount() const { return rayTracingProperties_.maxTriangleCount; }
        uint32_t ShaderGroupBaseAlignment() const { return rayTracingProperties_.shaderGroupBaseAlignment; }
        uint32_t ShaderGroupHandleSize() const { return rayTracingProperties_.shaderGroupHandleSize; }

    private:
        VkPhysicalDeviceRayTracingPropertiesKHR rayTracingProperties_{};
    };

    class RayTracingPropertiesLocator
    {
    public:
        static RayTracingProperties &GetProperties() { return *props_; };

        static void Provide(RayTracingProperties *props) { props_ = props; };

    private:
        static RayTracingProperties *props_;
    };

} // namespace estun
