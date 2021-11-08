#include "VulkanFramebuffer.h"
#include "VulkanUtils.h"
#include <iostream>
VulkanFramebuffer::VulkanFramebuffer(const VkDevice &device,
                                     const VkRenderPass &renderPass,
                                     const std::vector<VkImageView> &attachments,
                                     uint32_t width,
                                     uint32_t height)
    : m_TmpVKDeviceHandle(device)
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

    VK_CHECK(vkCreateFramebuffer(m_TmpVKDeviceHandle, &info, nullptr, &m_FramebufferHandle));
}
VulkanFramebuffer::~VulkanFramebuffer()
{
    vkDestroyFramebuffer(m_TmpVKDeviceHandle, m_FramebufferHandle, nullptr);
}
const VkFramebuffer &VulkanFramebuffer::GetVKFramebufferHandle() const
{
    return m_FramebufferHandle;
}
