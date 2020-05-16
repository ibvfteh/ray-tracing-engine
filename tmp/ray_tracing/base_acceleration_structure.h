#pragma once

#include "renderer/common.h"

namespace estun
{

    class BaseAccelerationStructure
    {
    public:
        struct MemoryRequirements
        {
            VkMemoryRequirements result;
            VkMemoryRequirements build;
            VkMemoryRequirements update;
        };

        BaseAccelerationStructure(const BaseAccelerationStructure &) = delete;
        BaseAccelerationStructure(BaseAccelerationStructure &&other) noexcept;

        BaseAccelerationStructure &operator=(const BaseAccelerationStructure &) = delete;
        BaseAccelerationStructure &operator=(BaseAccelerationStructure &&) = delete;

        virtual ~BaseAccelerationStructure();

        MemoryRequirements GetMemoryRequirements() const;

        static void ASMemoryBarrier(VkCommandBuffer commandBuffer);

        VkAccelerationStructureNV GetStructure() const;

    protected:
        BaseAccelerationStructure(const VkAccelerationStructureCreateInfoNV &createInfo);

        const bool allowUpdate_;

    private:
        VkAccelerationStructureNV accelerationStructure_;
    };

} // namespace estun
