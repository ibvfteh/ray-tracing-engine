#include "renderer/material/descriptor_set_layout.h"
#include "renderer/context/device.h"
#include "renderer/material/descriptor_binding.h"
#include <memory>
#include <vector>

estun::DescriptorSetLayout::DescriptorSetLayout(const std::vector<DescriptorBinding> &descriptorBindings) 
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    for (const auto &binding : descriptorBindings)
    {
        VkDescriptorSetLayoutBinding b = {};
        b.binding = binding.binding_;
        b.descriptorCount = binding.descriptorCount_;
        b.descriptorType = binding.type_;
        b.stageFlags = binding.stage_;

        layoutBindings.push_back(b);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(DeviceLocator::GetLogicalDevice(), &layoutInfo, nullptr, &layout), "create descriptor set layout");
}

estun::DescriptorSetLayout::~DescriptorSetLayout()
{
    if (layout != nullptr)
    {
        vkDestroyDescriptorSetLayout(DeviceLocator::GetLogicalDevice(), layout, nullptr);
        layout = nullptr;
    }
}

VkDescriptorSetLayout estun::DescriptorSetLayout::GetLayout() const
{
    return layout;
}
