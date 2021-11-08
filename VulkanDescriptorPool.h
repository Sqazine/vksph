#pragma once
#include <vulkan/vulkan.h>
class VulkanDescriptorPool
{
public:
    VulkanDescriptorPool(const VkDevice& device,uint32_t descriptorCount);
    ~VulkanDescriptorPool();

    const VkDescriptorPool& GetVKDescriptorPoolHandle() const;
private:
    VkDescriptorPool m_DescriptorPoolHandle;
    const VkDevice &m_TmpVKDeviceHandle;
};