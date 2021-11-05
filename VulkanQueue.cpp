#include "VulkanQueue.h"
#include "VulkanDevice.h"
VulkanQueue::VulkanQueue(const VulkanDevice *device, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    vkGetDeviceQueue(device->GetLogicalDeviceHandle(), queueFamilyIndex, queueIndex, &m_QueueHandle);
}
VulkanQueue::~VulkanQueue()
{
}

VkResult VulkanQueue::Submit(uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) const
{
    return vkQueueSubmit(m_QueueHandle, submitCount, pSubmits, fence);
}
VkResult VulkanQueue::Present(const VkPresentInfoKHR *pPresentInfo) const
{
    return vkQueuePresentKHR(m_QueueHandle, pPresentInfo);
}
VkResult VulkanQueue::WaitIdle() const
{
    return vkQueueWaitIdle(m_QueueHandle);
}

const VkQueue &VulkanQueue::GetVKQueueHandle() const
{
    return m_QueueHandle;
}