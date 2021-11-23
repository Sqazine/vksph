#include "SwapChain.h"
#include "Utils.h"
#include <iostream>
namespace VK
{
    SwapChain::SwapChain(SDL_Window *window, const Instance *instance, const Device *device)
        : m_TmpDevice(device)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device->GetPhysicalDeviceHandle(), instance->GetVKSurfaceKHRHandle());
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapChainExtent(window, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = instance->GetVKSurfaceKHRHandle();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (!device->GetQueueIndices().IsSameFamily())
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = device->GetQueueIndices().FamilyIndexArray().size();
            createInfo.pQueueFamilyIndices = device->GetQueueIndices().FamilyIndexArray().data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(m_TmpDevice->GetLogicalDeviceHandle(), &createInfo, nullptr, &m_SwapChainHandle));

        imageCount = 0;
        vkGetSwapchainImagesKHR(m_TmpDevice->GetLogicalDeviceHandle(), m_SwapChainHandle, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_TmpDevice->GetLogicalDeviceHandle(), m_SwapChainHandle, &imageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.image = m_SwapChainImages[i];
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = m_SwapChainImageFormat;
            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_SwapChainImageViews[i]));
        }
    }
    SwapChain::~SwapChain()
    {
        for (auto imageView : m_SwapChainImageViews)
            vkDestroyImageView(m_TmpDevice->GetLogicalDeviceHandle(), imageView, nullptr);
        vkDestroySwapchainKHR(m_TmpDevice->GetLogicalDeviceHandle(), m_SwapChainHandle, nullptr);
    }

    const VkSwapchainKHR &SwapChain::GetVKSwapChainHandle() const
    {
        return m_SwapChainHandle;
    }
    const std::vector<VkImage> &SwapChain::GetVKSwapChainImages() const
    {
        return m_SwapChainImages;
    }
    const std::vector<VkImageView> &SwapChain::GetVKSwapChainImageViews() const
    {
        return m_SwapChainImageViews;
    }

    const VkFormat &SwapChain::GetVKSwapChainImageFormat() const
    {
        return m_SwapChainImageFormat;
    }
    const VkExtent2D &SwapChain::GetVKSwapChainExtent() const
    {
        return m_SwapChainExtent;
    }
}