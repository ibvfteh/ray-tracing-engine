#pragma once

#include "renderer/common.h"
#include "renderer/material/push_constant.h"
#include "renderer/material/descriptor_sets.h"
#include "renderer/material/pipeline_layout.h"

namespace estun
{

    class DescriptorBinding;
    class DescriptorSets;
    class DescriptorPool;
    class DescriptorSetLayout;
    class TopLevelAccelerationStructure;
    class Buffer;
    class Texture;

    class Descriptor
    {
    public:
        Descriptor(const Descriptor &) = delete;
        Descriptor(Descriptor &&) = delete;

        Descriptor &operator=(const Descriptor &) = delete;
        Descriptor &operator=(Descriptor &&) = delete;

        explicit Descriptor(const std::vector<DescriptorBinding> &descriptorBindings, size_t maxSets);
        ~Descriptor();

        void Bind(VkCommandBuffer &commandBuffer, VkPipelineBindPoint point);

        template <typename T>
        void AddPushConstants(PushConstant<T> &constant)
        {
            VkPushConstantRange range = {};
            range.stageFlags = constant.stageFlags_;
            range.offset = 0;
            range.size = constant.GetSize();

            pipelineLayout.reset(new PipelineLayout(*descriptorSetLayout, range, true));
        }

        DescriptorSetLayout &GetDescriptorSetLayout() const;
        DescriptorSets &GetDescriptorSets();
        PipelineLayout &GetPipelineLayout() const;

    private:
        std::unique_ptr<DescriptorPool> descriptorPool;
        std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
        std::unique_ptr<DescriptorSets> descriptorSets;
        std::unique_ptr<PipelineLayout> pipelineLayout;
    };

} // namespace estun