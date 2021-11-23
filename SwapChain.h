#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Instance.h"
#include "Device.h"
namespace VK
{

    class SwapChain
    {
    public:
        SwapChain(SDL_Window *window, const Instance *instance, const Device *device);
        ~SwapChain();

        const VkSwapchainKHR &GetVKSwapChainHandle() const;
        const std::vector<VkImage> &GetVKSwapChainImages() const;
        const std::vector<VkImageView> &GetVKSwapChainImageViews() const;

        const VkFormat &GetVKSwapChainImageFormat() const;
        const VkExtent2D &GetVKSwapChainExtent() const;

    private:
        VkSwapchainKHR m_SwapChainHandle;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;

        const Device *m_TmpDevice;
    };

}