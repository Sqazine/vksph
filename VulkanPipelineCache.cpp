#include "VulkanPipelineCache.h"
#include <iostream>
#include "VulkanUtils.h"
VulkanPipelineCache::VulkanPipelineCache(const VkDevice &device, size_t initialDataSize, void *initialData)
    : m_TmpVKDeviceHandle(device)
{
    VkPipelineCacheCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.initialDataSize = initialDataSize;
    info.pInitialData = initialData;

    VK_CHECK(vkCreatePipelineCache(m_TmpVKDeviceHandle, &info, nullptr, &m_PipelineCacheHandle));
}
VulkanPipelineCache::~VulkanPipelineCache()
{
    vkDestroyPipelineCache(m_TmpVKDeviceHandle, m_PipelineCacheHandle, nullptr);
}

const VkPipelineCache &VulkanPipelineCache::GetVKPipelineCacheHandle() const
{
    return m_PipelineCacheHandle;
}