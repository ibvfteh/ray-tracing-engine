#pragma once

#include "renderer/common.h"

namespace estun
{

class DescriptorBinding;

class DescriptorPool
{
public:
    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool(DescriptorPool &&) = delete;

    DescriptorPool &operator=(const DescriptorPool &) = delete;
    DescriptorPool &operator=(DescriptorPool &&) = delete;

    DescriptorPool(const std::vector<DescriptorBinding> &descriptorBindings, size_t maxSets);
    ~DescriptorPool();

    VkDescriptorPool GetDescriptorPool() const;

private:
    VkDescriptorPool descriptorPool;
};

} // namespace estun
