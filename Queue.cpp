#include "Queue.h"
#include "GraphicsContext.h"
#include <iostream>
#include "Utils.h"
#include "Fence.h"
#include "Device.h"
namespace VK
{
    Queue::Queue(const Device* device, uint32_t queueFamilyIndex, uint32_t queueIndex)
    {
        vkGetDeviceQueue(device->GetLogicalDeviceHandle(), queueFamilyIndex, queueIndex, &m_QueueHandle);
    }
    Queue::~Queue()
    {
    }

    void Queue::Submit(uint32_t submitCount, const VkSubmitInfo *pSubmits, Fence* fence) const
    {
        VK_CHECK(vkQueueSubmit(m_QueueHandle, submitCount, pSubmits, fence==nullptr?VK_NULL_HANDLE:fence->GetVKFenceHandle()));
    }
    void Queue::Present(const VkPresentInfoKHR *pPresentInfo) const
    {
        VK_CHECK(vkQueuePresentKHR(m_QueueHandle, pPresentInfo));
    }
    void Queue::WaitIdle() const
    {
        VK_CHECK(vkQueueWaitIdle(m_QueueHandle));
    }

    const VkQueue &Queue::GetVKQueueHandle() const
    {
        return m_QueueHandle;
    }
}