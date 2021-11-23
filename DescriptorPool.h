#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class DescriptorPool
    {
    public:
        DescriptorPool(const class Device* device, uint32_t descriptorCount);
        ~DescriptorPool();

        const VkDescriptorPool &GetVKDescriptorPoolHandle() const;

    private:
        VkDescriptorPool m_DescriptorPoolHandle;
        const class Device *m_TmpDevice;
    };
}