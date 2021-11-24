#include "DescriptorSetLayout.h"
#include "Utils.h"
#include <iostream>
#include "GraphicsContext.h"
namespace VK
{
    DescriptorSetLayout::DescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &setLayoutBindings)

    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.pNext = nullptr;
        descriptorSetLayoutInfo.flags = 0;
        descriptorSetLayoutInfo.bindingCount = setLayoutBindings.size();
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayoutHandle));
    }
    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_DescriptorSetLayoutHandle, nullptr);
    }

    const VkDescriptorSetLayout &DescriptorSetLayout::GetVKDescriptorSetLayoutHandle() const
    {
        return m_DescriptorSetLayoutHandle;
    }
}