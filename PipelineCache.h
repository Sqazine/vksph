#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class PipelineCache
    {
    public:
        PipelineCache(const class Device *device, size_t initialDataSize = 0, void *initialData = nullptr);
        ~PipelineCache();

        const VkPipelineCache &GetVKPipelineCacheHandle() const;

    private:
        VkPipelineCache m_PipelineCacheHandle;
        const Device *m_TmpDevice;
    };
}