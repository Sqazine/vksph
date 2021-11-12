#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipelineCache.h"
#include "VulkanUtils.h"
#include <iostream>
VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice *device, const VulkanShader *shader, const VulkanPipelineLayout *layout, const VulkanPipelineCache *pipelineCache)
    : m_TmpVulkanDevice(device)
{
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stage = shader->GetStageCreateInfo();
    pipelineInfo.layout = layout->GetVKPipelineLayoutHandle();
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateComputePipelines(m_TmpVulkanDevice->GetLogicalDeviceHandle(),
                                      pipelineCache == nullptr ? nullptr : pipelineCache->GetVKPipelineCacheHandle(),
                                      1,
                                      &pipelineInfo,
                                      nullptr,
                                      &m_PipelineHandle));
}
VulkanComputePipeline::~VulkanComputePipeline()
{
    vkDestroyPipeline(m_TmpVulkanDevice->GetLogicalDeviceHandle(), m_PipelineHandle, nullptr);
}

const VkPipeline &VulkanComputePipeline::GetVKPipelineHandle() const
{
    return m_PipelineHandle;
}