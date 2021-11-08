#pragma once
#include <vulkan/vulkan.h>
class VulkanBuffer
{
    private:
    VkBuffer m_BufferHandle; 
    VkBufferView m_BufferViewHandle;
};
