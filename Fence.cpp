#include "Fence.h"
#include <iostream>
#include "Utils.h"
#include "Device.h"
namespace VK
{
    Fence::Fence(const Device *device, VkFenceCreateFlags flags)
        : m_TmpDevice(device)
    {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;

        VK_CHECK(vkCreateFence(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_FenceHandle));
    }
    Fence::~Fence()
    {
        vkDestroyFence(m_TmpDevice->GetLogicalDeviceHandle(), m_FenceHandle, nullptr);
    }

    const VkFence &Fence::GetVKFenceHandle() const
    {
        return m_FenceHandle;
    }

    void Fence::Wait(bool waitAll, uint64_t timeout)
    {
        VK_CHECK(vkWaitForFences(m_TmpDevice->GetLogicalDeviceHandle(),1,&m_FenceHandle,waitAll,timeout));
    }

    void Fence::Reset()
    {
        VK_CHECK(vkResetFences(m_TmpDevice->GetLogicalDeviceHandle(),1,&m_FenceHandle));
    }
}