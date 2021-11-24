#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class Queue
    {
    public:
        Queue(const class Device* device, uint32_t queueFamilyIndex, uint32_t queueIndex);
        ~Queue();

        void Submit(uint32_t submitCount, const VkSubmitInfo *pSubmits,class Fence* fence=nullptr) const;
        void Present(const VkPresentInfoKHR *pPresentInfo) const;
        void WaitIdle() const;

        const VkQueue &GetVKQueueHandle() const;

    private:
        VkQueue m_QueueHandle;
    };
}