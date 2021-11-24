#include "Semaphore.h"
#include <iostream>
#include "Utils.h"
#include "GraphicsContext.h"
namespace VK
{
    Semaphore::Semaphore()
        
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        VK_CHECK(vkCreateSemaphore(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &info, nullptr, &m_SemaphoreHandle));
    }
    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_SemaphoreHandle, nullptr);
    }

    const VkSemaphore &Semaphore::GetVKSemaphoreHandle() const
    {
        return m_SemaphoreHandle;
    }
}