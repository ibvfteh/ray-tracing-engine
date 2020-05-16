#pragma once

#include "renderer/common.h"

namespace estun
{

class DescriptorBinding;

class DescriptorSetLayout
{
public:
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout(DescriptorSetLayout &&) = delete;

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

    DescriptorSetLayout(const std::vector<DescriptorBinding> &descriptorBindings);
    ~DescriptorSetLayout();

    VkDescriptorSetLayout GetLayout() const;

private:
    VkDescriptorSetLayout layout;
};

} // namespace estun