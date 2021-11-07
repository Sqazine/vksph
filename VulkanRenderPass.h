#pragma once
#include <vulkan/vulkan.h>
class VulkanRenderPass
{
public:
    VulkanRenderPass(const class VulkanDevice *device, const class VulkanSwapChain *swapChain);
    ~VulkanRenderPass();

    const VkRenderPass &GetVKRenderPassHandle() const;

private:
    VkRenderPass m_RenderPassHandle;

    const class VulkanDevice *m_TmpVulkanDevice;
};