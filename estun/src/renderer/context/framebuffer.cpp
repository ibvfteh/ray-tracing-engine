#include "renderer/context/framebuffer.h"
#include "renderer/context/device.h"
#include "renderer/context/render_pass.h"
#include "renderer/context/swap_chain.h"
#include "renderer/context/image_view.h"
#include "renderer/context.h"

estun::Framebuffer::Framebuffer(
    const std::vector<ImageView*> &attachments,
    const std::unique_ptr<RenderPass> &renderPass)
{
    
    std::vector<VkImageView> vkAttachments;
    for (auto & at : attachments)
    {
        vkAttachments.push_back(at->GetImageView());
    }

    if (attachments.size() == 0)
    {
        ES_CORE_ASSERT("Failed to create framebuffer, attachments are empty")
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->GetRenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = vkAttachments.data();
    framebufferInfo.width = ContextLocator::GetSwapChain()->GetExtent().width;
    framebufferInfo.height = ContextLocator::GetSwapChain()->GetExtent().height;
    framebufferInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(DeviceLocator::GetLogicalDevice(), &framebufferInfo, nullptr, &framebuffer), "Failed to create framebuffer");
}

estun::Framebuffer::Framebuffer(Framebuffer &&other) noexcept : framebuffer(other.framebuffer)
{
    other.framebuffer = nullptr;
}

estun::Framebuffer::~Framebuffer()
{
    if (framebuffer != nullptr)
    {
        vkDestroyFramebuffer(DeviceLocator::GetLogicalDevice(), framebuffer, nullptr);
        framebuffer = nullptr;
    }
}

VkFramebuffer estun::Framebuffer::GetFramebuffer() const
{
    return framebuffer;
}

/*
namespace estun
{
    VulkanFramebuffers::VulkanFramebuffers(
            VulkanImageView* imageView,
            VulkanRenderPass* renderPass,
            VkExtent2D* swapChainExtent,
            VkImageView* depthImageView,
            VkImageView* colorImageView
            )
    {
        const std::vector<VkImageView>* swapChainImageViews = imageView->GetSwapChainImageViewsVector();

        swapChainFramebuffers.resize(swapChainImageViews->size());

        for (size_t i = 0; i < swapChainImageViews->size(); i++) {
            std::array<VkImageView, 3> attachments = {
                *colorImageView,
                *depthImageView,
                (*swapChainImageViews)[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *renderPass->GetRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent->width;
            framebufferInfo.height = swapChainExtent->height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(*VulkanDeviceLocator::GetLogicalDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) 
            {
                ES_CORE_ASSERT("Failed to create framebuffer");
            }
        }
    }
    
    VulkanFramebuffers::~VulkanFramebuffers()
    {
        for (auto const framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(*VulkanDeviceLocator::GetLogicalDevice(), framebuffer, nullptr);
        }
    }

    std::vector<VkFramebuffer>* VulkanFramebuffers::GetSwapChainFramebuffersVector() 
    {
        return &swapChainFramebuffers;
    }

    size_t VulkanFramebuffers::GetSwapChainFramebuffersVectorSize() 
    {
        return swapChainFramebuffers.size();
    }
}
*/