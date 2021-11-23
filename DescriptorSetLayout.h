#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(const class Device *device, const std::vector<VkDescriptorSetLayoutBinding> &setLayoutBindings);
        ~DescriptorSetLayout();

        const VkDescriptorSetLayout &GetVKDescriptorSetLayoutHandle() const;

    private:
        VkDescriptorSetLayout m_DescriptorSetLayoutHandle;

        const class Device *m_TmpDevice;
    };
}