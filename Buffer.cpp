#include "Buffer.h"
#include "Utils.h"
#include "Device.h"
#include <iostream>
#include "GraphicsContext.h"
namespace VK
{
    Buffer::Buffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkSharingMode sharingMode,
        VkMemoryPropertyFlags properties)
        : Buffer(size, usage, sharingMode, 0, properties)
    {
    }

    Buffer::Buffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkSharingMode sharingMode,
        VkMemoryAllocateFlags allocateFlags,
        VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo bufferInfo;
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = sharingMode;

        VK_CHECK(vkCreateBuffer(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &bufferInfo, nullptr, &m_BufferHandle));

        VkMemoryRequirements memRequirements;
        memRequirements = GetMemoryRequirements();

        m_Size = memRequirements.size;

        VkMemoryAllocateFlagsInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.pNext = nullptr;
        flagsInfo.flags = allocateFlags;
        flagsInfo.deviceMask = -1;

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = GraphicsContext::GetDevice()->FindMemoryType(memRequirements.memoryTypeBits, properties);
        allocInfo.pNext = &flagsInfo;

        VK_CHECK(vkAllocateMemory(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &allocInfo, nullptr, &m_BufferMemoryHandle));

        vkBindBufferMemory(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferHandle, m_BufferMemoryHandle, 0);
    }
    Buffer::~Buffer()
    {
        vkFreeMemory(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferMemoryHandle, nullptr);
        vkDestroyBuffer(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferHandle, nullptr);
    }

    VkMemoryRequirements Buffer::GetMemoryRequirements() const
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferHandle, &memRequirements);
        return memRequirements;
    }

    const VkBuffer &Buffer::GetVKBufferHandle() const
    {
        return m_BufferHandle;
    }
    const VkDeviceMemory &Buffer::GetVKBufferMemory() const
    {
        return m_BufferMemoryHandle;
    }

    const VkDeviceSize &Buffer::GetVKBufferSize() const
    {
        return m_Size;
    }

    void Buffer::Fill(size_t size, const void *data)
    {
        void *mappedMemory = nullptr;
        vkMapMemory(VK::GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferMemoryHandle, 0, m_Size, 0, &mappedMemory);
        std::memset(mappedMemory, 0, size);
        std::memcpy(mappedMemory, data, size);
        vkUnmapMemory(VK::GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_BufferMemoryHandle);
    }

    void Buffer::CopyFrom(VkCommandBuffer commandBuffer, VkBufferCopy bufferCopy, const Buffer &buffer)
    {
        vkCmdCopyBuffer(commandBuffer, buffer.m_BufferHandle, m_BufferHandle, 1, &bufferCopy);
    }

}