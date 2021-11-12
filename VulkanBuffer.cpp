#include "VulkanBuffer.h"
#include "VulkanUtils.h"
#include "VulkanDevice.h"
#include <iostream>

VulkanBuffer::VulkanBuffer(const VulkanDevice *device,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkSharingMode sharingMode,
                           VkMemoryPropertyFlags properties)
    : VulkanBuffer(device, size, usage, sharingMode, 0, properties)
{
}

VulkanBuffer::VulkanBuffer(const VulkanDevice *device,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkSharingMode sharingMode,
                           VkMemoryAllocateFlags allocateFlags,
                           VkMemoryPropertyFlags properties)
    : m_TmpDevice(device)
{
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    VK_CHECK(vkCreateBuffer(m_TmpDevice->GetLogicalDeviceHandle(), &bufferInfo, nullptr, &m_BufferHandle));

    VkMemoryRequirements memRequirements;
    memRequirements = GetMemoryRequirements();

    VkMemoryAllocateFlagsInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    flagsInfo.pNext = nullptr;
    flagsInfo.flags = allocateFlags;
    flagsInfo.deviceMask = -1;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_TmpDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);
    allocInfo.pNext = &flagsInfo;

    VK_CHECK(vkAllocateMemory(m_TmpDevice->GetLogicalDeviceHandle(), &allocInfo, nullptr, &m_BufferMemoryHandle));

    vkBindBufferMemory(m_TmpDevice->GetLogicalDeviceHandle(), m_BufferHandle, m_BufferMemoryHandle, 0);
}
VulkanBuffer::~VulkanBuffer()
{
    vkFreeMemory(m_TmpDevice->GetLogicalDeviceHandle(), m_BufferMemoryHandle, nullptr);
    vkDestroyBuffer(m_TmpDevice->GetLogicalDeviceHandle(), m_BufferHandle, nullptr);
}

VkMemoryRequirements VulkanBuffer::GetMemoryRequirements() const
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_TmpDevice->GetLogicalDeviceHandle(), m_BufferHandle, &memRequirements);
    return memRequirements;
}

const VkBuffer &VulkanBuffer::GetVKBufferHandle() const
{
    return m_BufferHandle;
}
const VkDeviceMemory &VulkanBuffer::GetVKBufferMemory() const
{
    return m_BufferMemoryHandle;
}