#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
class VulkanSwapChain
{
public:
    VulkanSwapChain(SDL_Window *window, const VulkanInstance *instance, const VulkanDevice *device);
    ~VulkanSwapChain();

    const VkSwapchainKHR &GetVKSwapChainHandle() const;
    const std::vector<VkImage>& GetVKSwapChainImages() const;
    const std::vector<VkImageView>& GetVKSwapChainImageViews() const;

    const VkFormat& GetVKSwapChainImageFormat() const;
    const VkExtent2D& GetVKSwapChainExtent() const;
private:
    VkSwapchainKHR m_SwapChainHandle;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    const VulkanDevice *m_TmpVulkanDevice;
};