#pragma once

#include "renderer/common.h"

namespace estun
{

    class RayTracingProperties
    {
    public:
        explicit RayTracingProperties();

        uint32_t MaxDescriptorSetAccelerationStructures() const { return props_.maxDescriptorSetAccelerationStructures; }
        uint64_t MaxGeometryCount() const { return props_.maxGeometryCount; }
        uint64_t MaxInstanceCount() const { return props_.maxInstanceCount; }
        uint32_t MaxRecursionDepth() const { return props_.maxRecursionDepth; }
        uint32_t MaxShaderGroupStride() const { return props_.maxShaderGroupStride; }
        uint64_t MaxTriangleCount() const { return props_.maxTriangleCount; }
        uint32_t ShaderGroupBaseAlignment() const { return props_.shaderGroupBaseAlignment; }
        uint32_t ShaderGroupHandleSize() const { return props_.shaderGroupHandleSize; }

    private:
        VkPhysicalDeviceRayTracingPropertiesNV props_{};
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
