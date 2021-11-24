#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class Semaphore
    {
    public:
        Semaphore();
        ~Semaphore();

        const VkSemaphore &GetVKSemaphoreHandle() const;

    private:
        VkSemaphore m_SemaphoreHandle;
        
    };
}