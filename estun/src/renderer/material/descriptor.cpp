#include "renderer/material/descriptor.h"
#include "core/core.h"
#include "renderer/context.h"
#include "renderer/material/descriptor_binding.h"
#include "renderer/material/descriptor_sets.h"
#include "renderer/material/descriptor_pool.h"
#include "renderer/material/descriptor_set_layout.h"
#include "renderer/material/pipeline_layout.h"
//#include "renderer/ray_tracing/TopLevelAccelerationStructure.h"
#include "renderer/buffers/buffer.h"
#include "renderer/material/texture.h"

estun::Descriptor::Descriptor(const std::vector<DescriptorBinding> &descriptorBindings, const size_t maxSets)
{
    std::map<uint32_t, VkDescriptorType> bindingTypes;

    for (const auto &binding : descriptorBindings)
    {
        if (!bindingTypes.insert(std::make_pair(binding.binding_, binding.type_)).second)
        {
            ES_CORE_ASSERT("Binding collision");
        }
    }

    descriptorPool.reset(new DescriptorPool(descriptorBindings, maxSets));
    descriptorSetLayout.reset(new DescriptorSetLayout(descriptorBindings));
    descriptorSets.reset(new DescriptorSets(*descriptorPool, *descriptorSetLayout, bindingTypes, maxSets));

    for (int index = 0; index < maxSets; index++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        std::vector<DescriptableInfo> infos;
        uint32_t size = 0;
        for (int binding = 0; binding < descriptorBindings.size(); binding++)
        {
            size += descriptorBindings[binding].descriptable_.size();
        }
        infos.reserve(size);

        for (int binding = 0; binding < descriptorBindings.size(); binding++)
        {
            for (int j = 0; j < descriptorBindings[binding].descriptable_.size(); j++)
            {
                infos.push_back(descriptorBindings[binding].descriptable_[j]->GetInfo());
                descriptorWrites.push_back(descriptorSets->Bind(index, descriptorBindings[binding].binding_, infos.back()));
            }
        }
        descriptorSets->UpdateDescriptors(index, descriptorWrites);
    }

    pipelineLayout.reset(new PipelineLayout(*descriptorSetLayout, {}));
}

estun::Descriptor::~Descriptor()
{
    pipelineLayout.reset();
    descriptorSets.reset();
    descriptorSetLayout.reset();
    descriptorPool.reset();
}

estun::DescriptorSetLayout &estun::Descriptor::GetDescriptorSetLayout() const
{
    return *descriptorSetLayout;
}

estun::DescriptorSets &estun::Descriptor::GetDescriptorSets()
{
    return *descriptorSets;
}

estun::PipelineLayout &estun::Descriptor::GetPipelineLayout() const
{
    return *pipelineLayout;
}

void estun::Descriptor::Bind(VkCommandBuffer &commandBuffer,  VkPipelineBindPoint point)
{
    VkDescriptorSet vkDescriptorSets[] = {descriptorSets->GetDescriptorSet(ContextLocator::GetImageIndex())};
    vkCmdBindDescriptorSets(
        commandBuffer,
        point,
        pipelineLayout->GetPipelineLayout(),
        0, 1,
        vkDescriptorSets,
        0, nullptr);
}
