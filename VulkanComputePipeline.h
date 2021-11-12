#pragma once
#include <vulkan/vulkan.h>
class VulkanComputePipeline
{
public:
    VulkanComputePipeline(const class VulkanDevice *device, const class VulkanShader *shader, const class VulkanPipelineLayout *layout ,const class VulkanPipelineCache* pipelineCache=nullptr);
    ~VulkanComputePipeline();

    const VkPipeline &GetVKPipelineHandle() const;

private:
    VkPipeline m_PipelineHandle;
    const class VulkanDevice *m_TmpVulkanDevice;
};