#include "VulkanShader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "VulkanUtils.h"
VulkanShader::VulkanShader(const VkDevice &device, VkShaderStageFlagBits stageFlag, std::string_view filePath)
    : m_TmpVKDeviceHandle(device)
{
    std::ifstream file(filePath.data(), std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "failed to load shader file:" << filePath << std::endl;
        exit(1);
    }
    std::stringstream sstream;
    sstream << file.rdbuf();
    std::string content = sstream.str();
    file.close();

    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.flags = 0;
    info.pNext = nullptr;
    info.codeSize = content.size();
    info.pCode = reinterpret_cast<const uint32_t *>(content.data());

    VK_CHECK(vkCreateShaderModule(m_TmpVKDeviceHandle, &info, nullptr, &m_ShaderModuleHandle));

    m_StageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_StageCreateInfo.pNext = nullptr;
    m_StageCreateInfo.flags = 0;
    m_StageCreateInfo.stage = stageFlag;
    m_StageCreateInfo.module = m_ShaderModuleHandle;
    m_StageCreateInfo.pName = "main";
    m_StageCreateInfo.pSpecializationInfo = nullptr;
}
VulkanShader::~VulkanShader()
{
    vkDestroyShaderModule(m_TmpVKDeviceHandle, m_ShaderModuleHandle, nullptr);
}

const VkPipelineShaderStageCreateInfo &VulkanShader::GetStageCreateInfo() const
{
    return m_StageCreateInfo;
}
