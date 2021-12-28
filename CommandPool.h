#pragma once
#include <vulkan/vulkan.h>
#include "Utils.h"

namespace VK
{
    class CommandPool
    {
    public:
        CommandPool(uint32_t queueFamilyIndex);
        ~CommandPool();

        const VkCommandPool &GetVKCommandPoolHandle() const;

    private:
        VkCommandPool m_CommandPoolHandle;
    };

}