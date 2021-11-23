#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class Fence
    {
    public:
        Fence(const class Device *device,VkFenceCreateFlags flags);
        ~Fence();

        const VkFence &GetVKFenceHandle() const;

        void Wait(bool waitAll,uint64_t timeout);
        void Reset();

    private:
        VkFence m_FenceHandle;
        const class Device *m_TmpDevice;
    };
}