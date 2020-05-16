#pragma once

#include "renderer/common.h"

namespace estun
{

class DescriptorPool;
class DescriptorSetLayout;
class DescriptableInfo;

class DescriptorSets
{
public:
	DescriptorSets(const DescriptorSets &) = delete;
	DescriptorSets(DescriptorSets &&) = delete;

	DescriptorSets &operator=(const DescriptorSets &) = delete;
	DescriptorSets &operator=(DescriptorSets &&) = delete;

	DescriptorSets(
		const DescriptorPool &descriptorPool,
		const DescriptorSetLayout &layout,
		std::map<uint32_t, VkDescriptorType> bindingTypes,
		size_t size);

	~DescriptorSets();

	VkDescriptorSet GetDescriptorSet(uint32_t index) const;

	VkWriteDescriptorSet Bind(uint32_t index, uint32_t binding, const DescriptableInfo &info, uint32_t count = 1) const;

	void UpdateDescriptors(uint32_t index, const std::vector<VkWriteDescriptorSet> &descriptorWrites);

private:
	VkDescriptorType GetBindingType(uint32_t binding) const;

	const DescriptorPool &descriptorPool;
	const std::map<uint32_t, VkDescriptorType> bindingTypes;

	std::vector<VkDescriptorSet> descriptorSets;
};

} // namespace estun
