#include "VulkanDescriptorSetLayout.h"
#include "VulkanUtils.h"
#include <iostream>
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VkDevice &device, const std::vector<VkDescriptorSetLayoutBinding> &setLayoutBindings)
    : m_TmpVKDeviceHandle(device)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0;
    descriptorSetLayoutInfo.bindingCount = setLayoutBindings.size();;
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_TmpVKDeviceHandle, &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayoutHandle));
}
VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_TmpVKDeviceHandle, m_DescriptorSetLayoutHandle, nullptr);
}

const VkDescriptorSetLayout &VulkanDescriptorSetLayout::GetVKDescriptorSetLayoutHandle() const
{
    return m_DescriptorSetLayoutHandle;
}
