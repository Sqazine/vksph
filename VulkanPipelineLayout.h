#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class VulkanPipelineLayout
{
public:
    VulkanPipelineLayout(const VkDevice &device, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts={}, const std::vector<VkPushConstantRange> &pushConstantRange={});
    ~VulkanPipelineLayout();

    const VkPipelineLayout& GetVKPipelineLayoutHandle() const;
private:
    VkPipelineLayout m_PipelineLayoutHandle;
    const VkDevice &m_TmpVKDeviceHandle;
};