#pragma once
#include <vulkan/vulkan.h>

class VulkanQueue
{
public:
    VulkanQueue(const class VulkanDevice *device, uint32_t queueFamilyIndex,uint32_t queueIndex);
    ~VulkanQueue();

    VkResult Submit(uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) const;
    VkResult Present(const VkPresentInfoKHR *pPresentInfo) const;
    VkResult WaitIdle() const;

    const VkQueue& GetVKQueueHandle() const;

private:
    VkQueue m_QueueHandle;
};