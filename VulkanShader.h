#pragma once
#include <vulkan/vulkan.h>
#include <string_view>
class VulkanShader
{
public:
    VulkanShader(const VkDevice &device, VkShaderStageFlagBits stageFlag, std::string_view filePath);
    ~VulkanShader();

    const VkPipelineShaderStageCreateInfo &GetStageCreateInfo() const;

private:
    VkShaderModule m_ShaderModuleHandle;
    VkPipelineShaderStageCreateInfo m_StageCreateInfo;

    const VkDevice &m_TmpVKDeviceHandle;
};
