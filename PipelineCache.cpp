#include "PipelineCache.h"
#include <iostream>
#include "Utils.h"
#include "GraphicsContext.h"
namespace VK
{
    PipelineCache::PipelineCache( size_t initialDataSize, void *initialData)
        
    {
        VkPipelineCacheCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.initialDataSize = initialDataSize;
        info.pInitialData = initialData;

        VK_CHECK(vkCreatePipelineCache(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &info, nullptr, &m_PipelineCacheHandle));
    }
    PipelineCache::~PipelineCache()
    {
        vkDestroyPipelineCache(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_PipelineCacheHandle, nullptr);
    }

    const VkPipelineCache &PipelineCache::GetVKPipelineCacheHandle() const
    {
        return m_PipelineCacheHandle;
    }
}