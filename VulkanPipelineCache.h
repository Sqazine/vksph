#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class VulkanPipelineCache
{
public:
    VulkanPipelineCache(const VkDevice &device, size_t initialDataSize=0, void* initialData = nullptr);
    ~VulkanPipelineCache();

    const VkPipelineCache &GetVKPipelineCacheHandle() const;

private:
    VkPipelineCache m_PipelineCacheHandle;
    const VkDevice &m_TmpVKDeviceHandle;
};