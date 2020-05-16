#pragma once

#include "renderer/common.h"

namespace estun
{

class CommandPool;

class CommandBuffers
{
public:
    CommandBuffers(const CommandBuffers &) = delete;
    CommandBuffers(CommandBuffers &&) = delete;

    CommandBuffers &operator=(const CommandBuffers &) = delete;
    CommandBuffers &operator=(CommandBuffers &&) = delete;

    CommandBuffers(CommandPool &commandPool, uint32_t size);
    ~CommandBuffers();

    uint32_t Size() const { return static_cast<uint32_t>(commandBuffers.size()); }
    VkCommandBuffer &operator[](const size_t i) { return commandBuffers[i]; }

    void Begin(size_t i);
    void End(size_t i);

private:
    const CommandPool &commandPool_;

    std::vector<VkCommandBuffer> commandBuffers;
};

} // namespace estun

/*
#define WORKGROUP_SIZE 1

namespace estun
{
    class VulkanCommandBuffers
    {
    private:
        std::vector<VkCommandBuffer> commandBuffers;
		VkRenderPassBeginInfo        renderPassInfo;
	    VulkanCommandPool*           currCommandPool;
    public:
        explicit VulkanCommandBuffers(VulkanCommandPool* commandPool, int16_t cb_size);
        ~VulkanCommandBuffers();

        void InitCommandBuffers(
            VulkanRenderPass* renderPass, 
            VkExtent2D* swapChainExtent, 
            VulkanFramebuffers* vkSwapChainFramebuffers);
        void BeginCommandBuffers();
        void EndCommandBuffers();
        void CloseCommandBuffers();
        void ResetCommandBuffers();
        void FreeCommandBuffers();

        void BindShader(std::shared_ptr<VulkanGraphicsPipeline> graphicsPipeline, VulkanDescriptorSets* descriptorSets);
        void BindShader(std::shared_ptr<VulkanComputePipeline> computePipeline, VulkanDescriptorSets* descriptorSets);
        void LoadDraw(VulkanVertexBuffer* vertexBuffer, VulkanIndexBuffer* indexBuffer);
        void LoadCompute(uint32_t x, uint32_t y);

        std::vector<VkCommandBuffer>* GetCommandBuffersVector();
    };
}
*/
