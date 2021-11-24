#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class RenderPass
    {
    public:
        RenderPass( const class SwapChain *swapChain);
        ~RenderPass();

        const VkRenderPass &GetVKRenderPassHandle() const;

    private:
        VkRenderPass m_RenderPassHandle;

        
    };
}