#include "Framebuffer.h"
#include "Utils.h"
#include <iostream>
#include "Device.h"
namespace VK
{
    Framebuffer::Framebuffer(const Device *device,
                             const VkRenderPass &renderPass,
                             const std::vector<VkImageView> &attachments,
                             uint32_t width,
                             uint32_t height)
        : m_TmpDevice(device)
    {
        VkFramebufferCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.renderPass = renderPass;
        info.attachmentCount = attachments.size();
        info.pAttachments = attachments.data();
        info.width = width;
        info.height = height;
        info.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_FramebufferHandle));
    }
    Framebuffer::~Framebuffer()
    {
        vkDestroyFramebuffer(m_TmpDevice->GetLogicalDeviceHandle(), m_FramebufferHandle, nullptr);
    }
    const VkFramebuffer &Framebuffer::GetVKFramebufferHandle() const
    {
        return m_FramebufferHandle;
    }
}