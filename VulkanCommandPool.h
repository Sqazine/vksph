#pragma once
#include <vulkan/vulkan.h>
#include "VulkanUtils.h"
class VulkanCommandPool
{
public:
    VulkanCommandPool(const VkDevice &device, uint32_t queueFamilyIndex);
    ~VulkanCommandPool();

    const VkCommandPool &GetVKCommandPoolHandle() const;

    VkCommandBuffer AllocateCommandBuffer(VkCommandBufferLevel level);

    std::vector<VkCommandBuffer> AllocateCommandBuffers(uint32_t count,VkCommandBufferLevel level);

private:
    VkCommandPool m_CommandPoolHandle;

    DeletionQueue m_DeletionQueue;

    const VkDevice &m_TmpVKDeviceHandle;
};