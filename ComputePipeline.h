#pragma once
#include <vulkan/vulkan.h>
namespace VK
{
    class ComputePipeline
    {
    public:
        ComputePipeline( const class Shader *shader, const class PipelineLayout *layout, const class PipelineCache *pipelineCache = nullptr);
        ~ComputePipeline();

        const VkPipeline &GetVKPipelineHandle() const;

    private:
        VkPipeline m_PipelineHandle;
        
    };
}