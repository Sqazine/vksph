#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "CommandPool.h"
namespace VK
{
    class CommandBuffers
    {
    public:
        CommandBuffers(const CommandPool* commandPool,uint32_t count,VkCommandBufferLevel level=VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        ~CommandBuffers();

        const std::vector<VkCommandBuffer>& GetVKCommandBuffers() const;

        VkCommandBuffer Begin(size_t i);
    private:
        std::vector<VkCommandBuffer> m_CommandBuffers;
        const CommandPool* m_TmpCommandPool;
    };
}