#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class Framebuffer
    {
    public:
        Framebuffer(const VkRenderPass &renderPass, const std::vector<VkImageView> &attachments, uint32_t width, uint32_t height);
        ~Framebuffer();
        const VkFramebuffer &GetVKFramebufferHandle() const;

    private:
        VkFramebuffer m_FramebufferHandle;
    };
}