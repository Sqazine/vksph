#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Utils.h"
#include "Instance.h"
#include "Queue.h"
namespace VK
{
    class Device
    {
    public:
        Device(std::vector<const char *> neededDeviceExtensions);
        ~Device();

        void WaitIdle();

        const VkDevice &GetLogicalDeviceHandle() const;
        const VkPhysicalDevice &GetPhysicalDeviceHandle() const;
        const Queue *GetGraphicsQueue() const;
        const Queue *GetPresentQueue() const;
        const Queue *GetComputeQueue() const;

        QueueFamilyIndices GetQueueIndices() const;

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        VkPhysicalDevice SelectPhysicalDevice();

        VkPhysicalDevice m_PhysicalDeviceHandle;
        QueueFamilyIndices m_QueueIndices;
        VkDevice m_LogicalDeviceHandle;

        std::unique_ptr<Queue> m_GraphicsQueue;
        std::unique_ptr<Queue> m_PresentQueue;
        std::unique_ptr<Queue> m_ComputeQueue;

        std::vector<const char *> m_NeededDeviceExtensions;
    };
}