#include "renderer/material/vulkan_pipeline.h"
#include "renderer/material/vulkan_shader_manager.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/buffers/vulkan_vertex_buffer.h"

namespace estun
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
                const std::string vertexShaderName,
                const std::string fragmentShaderName,
                VulkanDescriptorSets* shaderDescriptorSets
                )
    {
        descriptorSets = shaderDescriptorSets;
        Init(vertexShaderName, fragmentShaderName);
    }

    void VulkanGraphicsPipeline::RebuildPipeline(
                const std::string vertexShaderName,
                const std::string fragmentShaderName
                )
    {
        Delete();
        Init(vertexShaderName, fragmentShaderName);
    }

    void VulkanGraphicsPipeline::Init(
                const std::string vertexShaderName,
                const std::string fragmentShaderName
                )
    {
        VkShaderModule vertShaderModule = ShaderManager::GetInstance()->GetShaderModule(vertexShaderName);
        VkShaderModule fragShaderModule = ShaderManager::GetInstance()->GetShaderModule(fragmentShaderName);

        VkExtent2D* swapChainExtent = VulkanContextLocator::GetContext()->GetSwapChain()->GetSwapChainExtent();

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName  = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName  = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription    = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount   = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x        = 0.0f;
        viewport.y        = (float) swapChainExtent->height;
        viewport.width    = (float) swapChainExtent->width;
	    viewport.height   = -(float) swapChainExtent->height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        ES_CORE_INFO(swapChainExtent->width);

        VkRect2D scissor = {};
        scissor.offset   = {0, 0};
        scissor.extent   = *swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE; 
        rasterizer.polygonMode             = VulkanContextLocator::GetContext()->GetPolygonMode();
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = VulkanDeviceLocator::GetDevice()->GetMsaaSamples();

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable       = VK_TRUE;
        depthStencil.depthWriteEnable      = VK_TRUE;
        depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable     = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable    = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = descriptorSets->GetDescriptorSetLayout();

        if (vkCreatePipelineLayout(*VulkanDeviceLocator::GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to create pipeline layout");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.layout              = pipelineLayout;
        pipelineInfo.renderPass          = *VulkanContextLocator::GetContext()->GetRenderPass()->GetRenderPass();
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(*VulkanDeviceLocator::GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to create graphics pipeline");
        }

        //vkDestroyShaderModule(*VulkanDeviceLocator::GetLogicalDevice(), fragShaderModule, nullptr);
        //vkDestroyShaderModule(*VulkanDeviceLocator::GetLogicalDevice(), vertShaderModule, nullptr);
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        Delete();
    }

    void VulkanGraphicsPipeline::Delete()
    {
        vkDestroyPipelineLayout(*VulkanDeviceLocator::GetLogicalDevice(), pipelineLayout, nullptr);
        vkDestroyPipeline(*VulkanDeviceLocator::GetLogicalDevice(), graphicsPipeline, nullptr);
    }

    VkPipeline* VulkanGraphicsPipeline::GetGraphicsPipeline()
    {
        return &graphicsPipeline;
    }

    VkPipelineLayout* VulkanGraphicsPipeline::GetPipelineLayout()
    {
        return &pipelineLayout;
    }

    VulkanDescriptorSets* VulkanGraphicsPipeline::GetDescriptorSets()
    {
        return descriptorSets;
    }

    VulkanComputePipeline::VulkanComputePipeline(
                const std::string computeShaderName,
                VulkanDescriptorSets* shaderDescriptorSets
                )
    {
        descriptorSets = shaderDescriptorSets;
        VkShaderModule computeShaderModule = ShaderManager::GetInstance()->GetShaderModule(computeShaderName);

        VkExtent2D* swapChainExtent = VulkanContextLocator::GetContext()->GetSwapChain()->GetSwapChainExtent();
      
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = computeShaderModule;
        shaderStageCreateInfo.pName  = "main";

        //VkPushConstantRange pcRange = {};   
        //pcRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        //pcRange.offset     = 0;
        //pcRange.size       = 2*sizeof(int);  

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount         = 1;
        pipelineLayoutCreateInfo.pSetLayouts            = descriptorSets->GetDescriptorSetLayout();
        //pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        //pipelineLayoutCreateInfo.pPushConstantRanges    = &pcRange;
        
        if (vkCreatePipelineLayout(*VulkanDeviceLocator::GetLogicalDevice(), &pipelineLayoutCreateInfo, NULL, &pipelineLayout)!= VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to create pipeline layout");
        }

        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage  = shaderStageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;

        if (vkCreateComputePipelines(*VulkanDeviceLocator::GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &computePipeline)!= VK_SUCCESS) 
        {
            ES_CORE_ASSERT("Failed to create compute pipeline");
        }
    
        //vkDestroyShaderModule(*VulkanDeviceLocator::GetLogicalDevice(), fragShaderModule, nullptr);
        //vkDestroyShaderModule(*VulkanDeviceLocator::GetLogicalDevice(), vertShaderModule, nullptr);
    }

    VulkanComputePipeline::~VulkanComputePipeline()
    {
        vkDestroyPipelineLayout(*VulkanDeviceLocator::GetLogicalDevice(), pipelineLayout, nullptr);
        vkDestroyPipeline(*VulkanDeviceLocator::GetLogicalDevice(), computePipeline, nullptr);
    }

    VkPipeline* VulkanComputePipeline::GetComputePipeline()
    {
        return &computePipeline;
    }

    VkPipelineLayout* VulkanComputePipeline::GetPipelineLayout()
    {
        return &pipelineLayout;
    }

    VulkanDescriptorSets* VulkanComputePipeline::GetDescriptorSets()
    {
        return descriptorSets;
    }
}
