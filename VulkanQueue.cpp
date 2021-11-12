#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include <iostream>
#include "VulkanUtils.h"
VulkanQueue::VulkanQueue(const VulkanDevice *device, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    vkGetDeviceQueue(device->GetLogicalDeviceHandle(), queueFamilyIndex, queueIndex, &m_QueueHandle);
}
VulkanQueue::~VulkanQueue()
{
}

void VulkanQueue::Submit(uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) const
{
    VK_CHECK(vkQueueSubmit(m_QueueHandle, submitCount, pSubmits, fence));
}
void VulkanQueue::Present(const VkPresentInfoKHR *pPresentInfo) const
{
    VK_CHECK( vkQueuePresentKHR(m_QueueHandle, pPresentInfo));
}
void VulkanQueue::WaitIdle() const
{
    VK_CHECK( vkQueueWaitIdle(m_QueueHandle));
}

const VkQueue &VulkanQueue::GetVKQueueHandle() const
{
    return m_QueueHandle;
}