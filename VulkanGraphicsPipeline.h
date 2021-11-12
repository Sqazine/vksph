#pragma once
#include <vulkan/vulkan.h>
#include <vector>
class VulkanGraphicsPipeline
{
public:
    VulkanGraphicsPipeline(const class VulkanDevice *device,
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
                           const class VulkanPipelineLayout* pipelineLayout,
                           const class VulkanRenderPass *renderPass,
                           const class VulkanPipelineCache* pipelineCache=nullptr);
    ~VulkanGraphicsPipeline();

    const VkPipeline &GetVKPipelineHandle() const;

private:
    VkPipeline m_PipelineHandle;
    const class VulkanDevice *m_TmpVulkanDevice;
};