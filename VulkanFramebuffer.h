#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class VulkanFramebuffer
{
public:
    VulkanFramebuffer(const VkDevice& device,const VkRenderPass& renderPass,const std::vector<VkImageView>& attachments,uint32_t width,uint32_t height);
    ~VulkanFramebuffer();
    const VkFramebuffer &GetVKFramebufferHandle() const;

private:
    VkFramebuffer m_FramebufferHandle;

    const VkDevice& m_TmpVKDeviceHandle;
};