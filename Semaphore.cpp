#include "Semaphore.h"
#include <iostream>
#include "Utils.h"
#include "Device.h"
namespace VK
{
    Semaphore::Semaphore(const Device *device)
        : m_TmpDevice(device)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        VK_CHECK(vkCreateSemaphore(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_SemaphoreHandle));
    }
    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(m_TmpDevice->GetLogicalDeviceHandle(), m_SemaphoreHandle, nullptr);
    }

    const VkSemaphore &Semaphore::GetVKSemaphoreHandle() const
    {
        return m_SemaphoreHandle;
    }
}