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

    private:
        VkMemoryRequirements GetMemoryRequirements() const;

        VkBuffer m_BufferHandle;
        VkDeviceMemory m_BufferMemoryHandle;
    };
}