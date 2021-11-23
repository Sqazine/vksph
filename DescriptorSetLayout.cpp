#include "DescriptorSetLayout.h"
#include "Utils.h"
#include <iostream>
#include "Device.h"
namespace VK
{
    DescriptorSetLayout::DescriptorSetLayout(const Device *device, const std::vector<VkDescriptorSetLayoutBinding> &setLayoutBindings)
        : m_TmpDevice(device)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.pNext = nullptr;
        descriptorSetLayoutInfo.flags = 0;
        descriptorSetLayoutInfo.bindingCount = setLayoutBindings.size();
        ;
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_TmpDevice->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayoutHandle));
    }
    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_TmpDevice->GetLogicalDeviceHandle(), m_DescriptorSetLayoutHandle, nullptr);
    }

    const VkDescriptorSetLayout &DescriptorSetLayout::GetVKDescriptorSetLayoutHandle() const
    {
        return m_DescriptorSetLayoutHandle;
    }
}