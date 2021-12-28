#pragma once
#include <vulkan/vulkan.h>

namespace VK
{
    class Buffer
    {
    public:
        Buffer(VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkSharingMode sharingMode,
               VkMemoryPropertyFlags properties);
               
        Buffer(VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkSharingMode sharingMode,
               VkMemoryAllocateFlags allocateFlags,
               VkMemoryPropertyFlags properties);

        ~Buffer();

        const VkBuffer &GetVKBufferHandle() const;
        const VkDeviceMemory &GetVKBufferMemory() const;
        const VkDeviceSize &GetVKBufferSize() const;

        void Fill(size_t size,const void* data);

        void CopyFrom(VkCommandBuffer commandBuffer, VkBufferCopy bufferCopy,const Buffer& buffer);

    private:
        VkMemoryRequirements GetMemoryRequirements() const;

        VkBuffer m_BufferHandle;
        VkDeviceMemory m_BufferMemoryHandle;
        VkDeviceSize m_Size;
    };
}