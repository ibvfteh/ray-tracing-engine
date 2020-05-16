#pragma once

#include "renderer/common.h"

namespace estun
{

struct MemoryRequirements
{
    VkMemoryRequirements result;
    VkMemoryRequirements build;
    VkMemoryRequirements update;
};

class BaseAccelerationStructure
{
public:
    BaseAccelerationStructure(const BaseAccelerationStructure &) = delete;
    BaseAccelerationStructure(BaseAccelerationStructure &&other) noexcept;
    
    BaseAccelerationStructure &operator=(const BaseAccelerationStructure &) = delete;
    BaseAccelerationStructure &operator=(BaseAccelerationStructure &&) = delete;


    virtual ~BaseAccelerationStructure();

    MemoryRequirements GetMemoryRequirements() const;
    void BindAccelerationMemory(VkDeviceMemory memory) ;

    static void ASMemoryBarrier(VkCommandBuffer commandBuffer);

    VkAccelerationStructureKHR GetStructure() const;

protected:
    BaseAccelerationStructure(const VkAccelerationStructureCreateInfoKHR &createInfo);

    const bool allowUpdate_;

private:
    VkAccelerationStructureKHR accelerationStructure_;
};

} // namespace estun
