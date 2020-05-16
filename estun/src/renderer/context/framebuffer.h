#pragma once

#include "renderer/common.h"

namespace estun
{

class RenderPass;
class ImageView;

class Framebuffer
{
public:
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) = delete;

    explicit Framebuffer(
        const std::vector<ImageView*> &attachments,
        const std::unique_ptr<RenderPass> &renderPass);
    Framebuffer(Framebuffer &&other) noexcept;
    ~Framebuffer();

    VkFramebuffer GetFramebuffer() const;

private:
    VkFramebuffer framebuffer;
};

} // namespace estun