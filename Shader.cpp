#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Device.h"
namespace VK
{
    Shader::Shader(const Device *device, VkShaderStageFlagBits stageFlag, std::string_view filePath)
        : m_TmpDevice(device)
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

        VK_CHECK(vkCreateShaderModule(m_TmpDevice->GetLogicalDeviceHandle(), &info, nullptr, &m_ShaderModuleHandle));

        m_StageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        m_StageCreateInfo.pNext = nullptr;
        m_StageCreateInfo.flags = 0;
        m_StageCreateInfo.stage = stageFlag;
        m_StageCreateInfo.module = m_ShaderModuleHandle;
        m_StageCreateInfo.pName = "main";
        m_StageCreateInfo.pSpecializationInfo = nullptr;
    }
    Shader::~Shader()
    {
        vkDestroyShaderModule(m_TmpDevice->GetLogicalDeviceHandle(), m_ShaderModuleHandle, nullptr);
    }

    const VkPipelineShaderStageCreateInfo &Shader::GetStageCreateInfo() const
    {
        return m_StageCreateInfo;
    }

}