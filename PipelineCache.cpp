#include "PipelineCache.h"
#include <iostream>
#include "Utils.h"
#include "Device.h"
namespace VK
{
    PipelineCache::PipelineCache(const Device *device, size_t initialDataSize, void *initialData)
        : m_TmpDevice(device)
    {
        VkPipelineCacheCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.initialDataSize = initialDataSize;
        info.pInitialData = initialData;

        VK_CHECK(vkCreatePipelineCache(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_PipelineCacheHandle));
    }
    PipelineCache::~PipelineCache()
    {
        vkDestroyPipelineCache(m_TmpDevice->GetLogicalDeviceHandle(), m_PipelineCacheHandle, nullptr);
    }

    const VkPipelineCache &PipelineCache::GetVKPipelineCacheHandle() const
    {
        return m_PipelineCacheHandle;
    }
}