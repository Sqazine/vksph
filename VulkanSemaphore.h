#pragma once
#include <vulkan/vulkan.h>
class VulkanSemaphore
{
public:
    VulkanSemaphore(const VkDevice &device);
    ~VulkanSemaphore();

    const VkSemaphore &GetVKSemaphoreHandle() const;

private:
    VkSemaphore m_SemaphoreHandle;
    const VkDevice &m_TmpVkDevice;
};