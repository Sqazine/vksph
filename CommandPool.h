#pragma once
#include <vulkan/vulkan.h>
#include "Utils.h"

namespace VK
{
    class CommandPool
    {
    public:
        CommandPool(const class Device *device, uint32_t queueFamilyIndex);
        ~CommandPool();

        const VkCommandPool &GetVKCommandPoolHandle() const;

        VkCommandBuffer AllocateCommandBuffer(VkCommandBufferLevel level);

        std::vector<VkCommandBuffer> AllocateCommandBuffers(uint32_t count, VkCommandBufferLevel level);

    private:
        VkCommandPool m_CommandPoolHandle;

        DeletionQueue m_DeletionQueue;

        const class Device *m_TmpDevice;
    };

}