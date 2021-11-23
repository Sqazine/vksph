#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace VK
{
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(const class Device *device,
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
                         const class PipelineLayout *pipelineLayout,
                         const class RenderPass *renderPass,
                         const class PipelineCache *pipelineCache = nullptr);
        ~GraphicsPipeline();

        const VkPipeline &GetVKPipelineHandle() const;

    private:
        VkPipeline m_PipelineHandle;
        const class Device *m_TmpDevice;
    };
}