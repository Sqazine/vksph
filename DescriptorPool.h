#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class DescriptorPool
    {
    public:
        DescriptorPool(const VkDescriptorPoolSize& poolSize);
        DescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~DescriptorPool();

        const VkDescriptorPool &GetVKDescriptorPoolHandle() const;

    private:
        VkDescriptorPool m_DescriptorPoolHandle;
    };
}