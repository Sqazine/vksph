#include "VulkanSemaphore.h"
#include <iostream>
#include "VulkanUtils.h"
VulkanSemaphore::VulkanSemaphore(const VkDevice &device)
    : m_TmpVkDevice(device)
{
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    VK_CHECK(vkCreateSemaphore(m_TmpVkDevice, &info, nullptr, &m_SemaphoreHandle));
}
VulkanSemaphore::~VulkanSemaphore()
{
    vkDestroySemaphore(m_TmpVkDevice, m_SemaphoreHandle, nullptr);
}

const VkSemaphore &VulkanSemaphore::GetVKSemaphoreHandle() const
{
    return m_SemaphoreHandle;
}