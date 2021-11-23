#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class PipelineLayout
    {
    public:
        PipelineLayout(const class Device *device, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts = {}, const std::vector<VkPushConstantRange> &pushConstantRange = {});
        ~PipelineLayout();

        const VkPipelineLayout &GetVKPipelineLayoutHandle() const;

    private:
        VkPipelineLayout m_PipelineLayoutHandle;
        const class Device *m_TmpDevice;
    };
}