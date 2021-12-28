#include "CommandPool.h"
#include "Utils.h"
#include <iostream>
#include "GraphicsContext.h"
namespace VK
{

    CommandPool::CommandPool(uint32_t queueFamilyIndex)
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = queueFamilyIndex;

        VK_CHECK(vkCreateCommandPool(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &info, nullptr, &m_CommandPoolHandle));
    }
    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_CommandPoolHandle, nullptr);
    }

    const VkCommandPool &CommandPool::GetVKCommandPoolHandle() const
    {
        return m_CommandPoolHandle;
    }
}