#include "VulkanGraphicsPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipelineCache.h"
#include "VulkanUtils.h"
#include <iostream>
VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanDevice *device,
                                               std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos,
                                               const VkPipelineVertexInputStateCreateInfo *pVertexInputState,
                                               const VkPipelineInputAssemblyStateCreateInfo *pInputAssemblyState,
                                               const VkPipelineTessellationStateCreateInfo *pTessellationState,
                                               const VkPipelineViewportStateCreateInfo *pViewportState,
                                               const VkPipelineRasterizationStateCreateInfo *pRasterizationState,
                                               const VkPipelineMultisampleStateCreateInfo *pMultisampleState,
                                               const VkPipelineDepthStencilStateCreateInfo *pDepthStencilState,
                                               const VkPipelineColorBlendStateCreateInfo *pColorBlendState,
                                               const VkPipelineDynamicStateCreateInfo *pDynamicState,
                                               const VulkanPipelineLayout *pipelineLayout,
                                               const VulkanRenderPass *renderPass,
                                               const VulkanPipelineCache *pipelineCache) : m_TmpVulkanDevice(device)
{
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
    graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos.data();
    graphicsPipelineCreateInfo.pVertexInputState = pVertexInputState;
    graphicsPipelineCreateInfo.pInputAssemblyState = pInputAssemblyState;
    graphicsPipelineCreateInfo.pTessellationState = nullptr;
    graphicsPipelineCreateInfo.pViewportState = pViewportState;
    graphicsPipelineCreateInfo.pRasterizationState = pRasterizationState;
    graphicsPipelineCreateInfo.pMultisampleState = pMultisampleState;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = pColorBlendState;
    graphicsPipelineCreateInfo.pDynamicState = nullptr;
    graphicsPipelineCreateInfo.layout = pipelineLayout->GetVKPipelineLayoutHandle();
    graphicsPipelineCreateInfo.renderPass = renderPass->GetVKRenderPassHandle();
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_TmpVulkanDevice->GetLogicalDeviceHandle(), pipelineCache == nullptr ? nullptr : pipelineCache->GetVKPipelineCacheHandle(), 1, &graphicsPipelineCreateInfo, nullptr, &m_PipelineHandle));
}
VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    vkDestroyPipeline(m_TmpVulkanDevice->GetLogicalDeviceHandle(), m_PipelineHandle, nullptr);
}

const VkPipeline &VulkanGraphicsPipeline::GetVKPipelineHandle() const
{
    return m_PipelineHandle;
}
