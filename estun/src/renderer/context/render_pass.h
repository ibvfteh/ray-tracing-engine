#pragma once

#include "renderer/common.h"

namespace estun
{

class Framebuffer;

class RenderPass
{
private:
    VkRenderPass renderPass;

public:
    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&) = delete;

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

    RenderPass(bool msaa, VkImageLayout colorFinal = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkImageLayout depthFinal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    ~RenderPass();

    void Begin(Framebuffer &farmebuffer, VkCommandBuffer &commandBuffer);
    void End(VkCommandBuffer &commandBuffer);

    VkRenderPass GetRenderPass() const;
};

} // namespace estun
