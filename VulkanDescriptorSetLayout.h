#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class VulkanDescriptorSetLayout
{
public:
    VulkanDescriptorSetLayout(const VkDevice &device,const std::vector<VkDescriptorSetLayoutBinding>& setLayoutBindings);
    ~VulkanDescriptorSetLayout();

    const VkDescriptorSetLayout &GetVKDescriptorSetLayoutHandle() const;

private:
    VkDescriptorSetLayout m_DescriptorSetLayoutHandle;

    const VkDevice &m_TmpVKDeviceHandle;
};