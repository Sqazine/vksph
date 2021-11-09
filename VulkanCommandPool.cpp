#include "VulkanCommandPool.h"

#include <iostream>
VulkanCommandPool::VulkanCommandPool(const VkDevice &device, uint32_t queueFamilyIndex)
    : m_TmpVKDeviceHandle(device)
{
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queueFamilyIndex;

    VK_CHECK(vkCreateCommandPool(m_TmpVKDeviceHandle, &info, nullptr, &m_CommandPoolHandle));
}
VulkanCommandPool::~VulkanCommandPool()
{
    m_DeletionQueue.Flush();
    vkDestroyCommandPool(m_TmpVKDeviceHandle, m_CommandPoolHandle, nullptr);
}

const VkCommandPool &VulkanCommandPool::GetVKCommandPoolHandle() const
{
    return m_CommandPoolHandle;
}

VkCommandBuffer VulkanCommandPool::AllocateCommandBuffer(VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = m_CommandPoolHandle;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBufferHandle;

    VK_CHECK(vkAllocateCommandBuffers(m_TmpVKDeviceHandle, &allocInfo, &commandBufferHandle));

    m_DeletionQueue.Add([=]()
                        { vkFreeCommandBuffers(m_TmpVKDeviceHandle, m_CommandPoolHandle, 1, &commandBufferHandle); });

    return commandBufferHandle;
}

std::vector<VkCommandBuffer> VulkanCommandPool::AllocateCommandBuffers(uint32_t count, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo bufferAllocInfo = {};
    bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocInfo.pNext = nullptr;
    bufferAllocInfo.commandBufferCount = count;
    bufferAllocInfo.commandPool = m_CommandPoolHandle;
    bufferAllocInfo.level = level;

    std::vector<VkCommandBuffer> commandBufferHandles(count);

    VK_CHECK(vkAllocateCommandBuffers(m_TmpVKDeviceHandle, &bufferAllocInfo, commandBufferHandles.data()));

    m_DeletionQueue.Add([=]()
                        { vkFreeCommandBuffers(m_TmpVKDeviceHandle, m_CommandPoolHandle, commandBufferHandles.size(), commandBufferHandles.data()); });
    
    return commandBufferHandles;
}