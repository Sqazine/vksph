#pragma once
#include <vulkan/vulkan.h>
#include <string_view>
namespace VK
{
    class Shader
    {
    public:
        Shader(const class Device *device, VkShaderStageFlagBits stageFlag, std::string_view filePath);
        ~Shader();

        const VkPipelineShaderStageCreateInfo &GetStageCreateInfo() const;

    private:
        VkShaderModule m_ShaderModuleHandle;
        VkPipelineShaderStageCreateInfo m_StageCreateInfo;

        const class Device *m_TmpDevice;
    };

}