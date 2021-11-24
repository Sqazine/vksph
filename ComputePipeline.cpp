#include "ComputePipeline.h"
#include "Device.h"
#include "Shader.h"
#include "PipelineLayout.h"
#include "PipelineCache.h"
#include "Utils.h"
#include <iostream>
#include "GraphicsContext.h"
namespace VK
{
    ComputePipeline::ComputePipeline( const Shader *shader, const PipelineLayout *layout, const PipelineCache *pipelineCache)
        
    {
        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;
        pipelineInfo.flags = 0;
        pipelineInfo.stage = shader->GetStageCreateInfo();
        pipelineInfo.layout = layout->GetVKPipelineLayoutHandle();
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VK_CHECK(vkCreateComputePipelines(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(),
                                          pipelineCache == nullptr ? nullptr : pipelineCache->GetVKPipelineCacheHandle(),
                                          1,
                                          &pipelineInfo,
                                          nullptr,
                                          &m_PipelineHandle));
    }
    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_PipelineHandle, nullptr);
    }

    const VkPipeline &ComputePipeline::GetVKPipelineHandle() const
    {
        return m_PipelineHandle;
    }
}