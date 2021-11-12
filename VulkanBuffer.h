#pragma once
#include <vulkan/vulkan.h>
class VulkanBuffer
{
public:
    VulkanBuffer(const class VulkanDevice *device,
                 VkDeviceSize size,
                 VkBufferUsageFlags usage,
                 VkSharingMode sharingMode,
                 VkMemoryPropertyFlags properties);

    VulkanBuffer(const class VulkanDevice *device,
                 VkDeviceSize size,
                 VkBufferUsageFlags usage,
                 VkSharingMode sharingMode,
                 VkMemoryAllocateFlags allocateFlags,
                 VkMemoryPropertyFlags properties);

    ~VulkanBuffer();

    const VkBuffer& GetVKBufferHandle() const;
    const VkDeviceMemory& GetVKBufferMemory() const;

private:
    VkMemoryRequirements GetMemoryRequirements() const;

    VkBuffer m_BufferHandle;
    VkDeviceMemory m_BufferMemoryHandle;
    const class VulkanDevice *m_TmpDevice;
};
