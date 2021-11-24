#include "Fence.h"
#include <iostream>
#include "Utils.h"
#include "GraphicsContext.h"
namespace VK
{
    Fence::Fence(VkFenceCreateFlags flags)
        
    {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;

        VK_CHECK(vkCreateFence(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &info, nullptr, &m_FenceHandle));
    }
    Fence::~Fence()
    {
        vkDestroyFence(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_FenceHandle, nullptr);
    }

    const VkFence &Fence::GetVKFenceHandle() const
    {
        return m_FenceHandle;
    }

    void Fence::Wait(bool waitAll, uint64_t timeout)
    {
        VK_CHECK(vkWaitForFences(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(),1,&m_FenceHandle,waitAll,timeout));
    }

    void Fence::Reset()
    {
        VK_CHECK(vkResetFences(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(),1,&m_FenceHandle));
    }
}