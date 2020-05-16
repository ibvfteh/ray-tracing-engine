#include "renderer/material/shader_module.h"
#include "renderer/context/device.h"
#include <fstream>

estun::ShaderModule::ShaderModule(const std::string& filename) :
	ShaderModule(ReadFile(filename))
{
}

estun::ShaderModule::ShaderModule(const std::vector<char>& code) 
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VK_CHECK_RESULT(vkCreateShaderModule(DeviceLocator::GetLogicalDevice(), &createInfo, nullptr, &shaderModule_), "create shader module");
}

estun::ShaderModule::~ShaderModule()
{
	if (shaderModule_ != nullptr)
	{
		vkDestroyShaderModule(DeviceLocator::GetLogicalDevice(), shaderModule_, nullptr);
		shaderModule_ = nullptr;
	}
}

VkPipelineShaderStageCreateInfo estun::ShaderModule::CreateShaderStage(VkShaderStageFlagBits stage) const
{
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = stage;
	createInfo.module = shaderModule_;
	createInfo.pName = "main";

	return createInfo;
}

std::vector<char> estun::ShaderModule::ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		ES_CORE_ASSERT("failed to open file '" + filename + "'");
	}

	const auto fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
