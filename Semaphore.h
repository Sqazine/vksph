#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class Semaphore
    {
    public:
        Semaphore(const class Device *device);
        ~Semaphore();

        const VkSemaphore &GetVKSemaphoreHandle() const;

    private:
        VkSemaphore m_SemaphoreHandle;
        const class Device *m_TmpDevice;
    };
}