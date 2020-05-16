#include "renderer/material/descriptor_sets.h"
#include "renderer/context/device.h"
#include "renderer/context/utils.h"
#include "renderer/material/descriptor_pool.h"
#include "renderer/material/descriptor_set_layout.h"
#include "renderer/material/descriptable.h"

#include <array>
#include <utility>

estun::DescriptorSets::DescriptorSets(
	const DescriptorPool &descriptorPool,
	const DescriptorSetLayout &layout,
	std::map<uint32_t, VkDescriptorType> bindingTypes,
	const size_t size)
	: descriptorPool(descriptorPool),
	  bindingTypes(std::move(bindingTypes))
{
	std::vector<VkDescriptorSetLayout> layouts(size, layout.GetLayout());

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool.GetDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(size);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(DeviceLocator::GetLogicalDevice(), &allocInfo, descriptorSets.data()), "Failed to allocate descriptor sets");
}

estun::DescriptorSets::~DescriptorSets()
{
	//if (!descriptorSets.empty())
	//{
	//	vkFreeDescriptorSets(
	//		descriptorPool_.Device().Handle(),
	//		descriptorPool_.Handle(),
	//		static_cast<uint32_t>(descriptorSets_.size()),
	//		descriptorSets_.data());

	//	descriptorSets.clear();
	//}
}

VkWriteDescriptorSet estun::DescriptorSets::Bind(const uint32_t index, const uint32_t binding, const DescriptableInfo &info, const uint32_t count) const
{
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSets[index];
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = GetBindingType(binding);
	descriptorWrite.descriptorCount = count;
	if (info.Collide())
	{
		ES_CORE_ASSERT("Colliding descriptable info");
	}
	bool hasValue = false;
	if (info.bI.has_value())
	{
		descriptorWrite.pBufferInfo = &info.bI.value();
		hasValue = true;
	}
	if (info.iI.has_value())
	{
		descriptorWrite.pImageInfo = &info.iI.value();
		hasValue = true;
	}
	if (info.asI.has_value())
	{
		descriptorWrite.pNext = &info.asI.value();
		hasValue = true;
	}
	if (!hasValue)
	{
		ES_CORE_ASSERT("No descriptable info");
	}

	return descriptorWrite;
}

void estun::DescriptorSets::UpdateDescriptors(uint32_t index, const std::vector<VkWriteDescriptorSet> &descriptorWrites)
{
	vkUpdateDescriptorSets(
		DeviceLocator::GetLogicalDevice(),
		static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}

VkDescriptorType estun::DescriptorSets::GetBindingType(uint32_t binding) const
{
	const auto it = bindingTypes.find(binding);
	if (it == bindingTypes.end())
	{
		ES_CORE_ASSERT("Binding not found");
	}

	return it->second;
}

VkDescriptorSet estun::DescriptorSets::GetDescriptorSet(uint32_t index) const
{
	return descriptorSets[index];
}
