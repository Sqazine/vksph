#include "CommandBuffers.h"
#include "Device.h"
#include "GraphicsContext.h"
#include "Utils.h"
#include <iostream>
namespace VK
{
    CommandBuffers::CommandBuffers(const CommandPool *commandPool, uint32_t count, VkCommandBufferLevel level)
        : m_TmpCommandPool(commandPool)
    {
        VkCommandBufferAllocateInfo bufferAllocInfo = {};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.pNext = nullptr;
        bufferAllocInfo.commandBufferCount = count;
        bufferAllocInfo.commandPool = commandPool->GetVKCommandPoolHandle();
        bufferAllocInfo.level = level;

        m_CommandBuffers.resize(count);

        VK_CHECK(vkAllocateCommandBuffers(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &bufferAllocInfo, m_CommandBuffers.data()));
    }
    CommandBuffers::~CommandBuffers()
    {
        vkFreeCommandBuffers(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_TmpCommandPool->GetVKCommandPoolHandle(), m_CommandBuffers.size(), m_CommandBuffers.data());
    }

    const std::vector<VkCommandBuffer> &CommandBuffers::GetVKCommandBuffers() const
    {
        return m_CommandBuffers;
    }

    VkCommandBuffer CommandBuffers::Begin(size_t i)
    {
        if (i >= m_CommandBuffers.size())
        {
            std::cout << "Out of command buffers size:" << i << std::endl;
            exit(1);
        }

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        info.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_CommandBuffers[i], &info);

        return m_CommandBuffers[i];
    }
}