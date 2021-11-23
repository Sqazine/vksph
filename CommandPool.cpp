#include "CommandPool.h"
#include "Utils.h"
#include <iostream>
#include "Device.h"
namespace VK
{

    CommandPool::CommandPool(const Device *device, uint32_t queueFamilyIndex)
        : m_TmpDevice(device)
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = queueFamilyIndex;

        VK_CHECK(vkCreateCommandPool(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_CommandPoolHandle));
    }
    CommandPool::~CommandPool()
    {
        m_DeletionQueue.Flush();
        vkDestroyCommandPool(m_TmpDevice->GetLogicalDeviceHandle(), m_CommandPoolHandle, nullptr);
    }

    const VkCommandPool &CommandPool::GetVKCommandPoolHandle() const
    {
        return m_CommandPoolHandle;
    }

    VkCommandBuffer CommandPool::AllocateCommandBuffer(VkCommandBufferLevel level)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.commandPool = m_CommandPoolHandle;
        allocInfo.level = level;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBufferHandle;

        VK_CHECK(vkAllocateCommandBuffers(m_TmpDevice->GetLogicalDeviceHandle(), &allocInfo, &commandBufferHandle));

        m_DeletionQueue.Add([=]()
                            { vkFreeCommandBuffers(m_TmpDevice->GetLogicalDeviceHandle(), m_CommandPoolHandle, 1, &commandBufferHandle); });

        return commandBufferHandle;
    }

    std::vector<VkCommandBuffer> CommandPool::AllocateCommandBuffers(uint32_t count, VkCommandBufferLevel level)
    {
        VkCommandBufferAllocateInfo bufferAllocInfo = {};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.pNext = nullptr;
        bufferAllocInfo.commandBufferCount = count;
        bufferAllocInfo.commandPool = m_CommandPoolHandle;
        bufferAllocInfo.level = level;

        std::vector<VkCommandBuffer> commandBufferHandles(count);

        VK_CHECK(vkAllocateCommandBuffers(m_TmpDevice->GetLogicalDeviceHandle(), &bufferAllocInfo, commandBufferHandles.data()));

        m_DeletionQueue.Add([=]()
                            { vkFreeCommandBuffers(m_TmpDevice->GetLogicalDeviceHandle(), m_CommandPoolHandle, commandBufferHandles.size(), commandBufferHandles.data()); });

        return commandBufferHandles;
    }
}