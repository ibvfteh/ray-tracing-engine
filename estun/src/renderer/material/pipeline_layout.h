#pragma once

#include "renderer/common.h"

namespace estun
{

class DescriptorSetLayout;

class PipelineLayout
{
public:
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) = delete;

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&) = delete;

    PipelineLayout(const DescriptorSetLayout &descriptorSetLayout, VkPushConstantRange pushConstantRange = {}, bool flag = false);
    ~PipelineLayout();

    VkPipelineLayout &GetPipelineLayout();

private:
    VkPipelineLayout pipelineLayout;
};

} // namespace estun