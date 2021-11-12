#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "VulkanUtils.h"
#include "VulkanInstance.h"
#include "VulkanQueue.h"
class VulkanDevice
{
public:
    VulkanDevice(const VulkanInstance* instance, std::vector<const char *> neededDeviceExtensions);
    ~VulkanDevice();

    void WaitIdle();

    const VkDevice& GetLogicalDeviceHandle() const;
    const VkPhysicalDevice& GetPhysicalDeviceHandle() const;
    const VulkanQueue* GetGraphicsQueue() const;
    const VulkanQueue* GetPresentQueue() const;
    const VulkanQueue* GetComputeQueue() const;

    QueueFamilyIndices GetQueueIndices() const;

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
private:
    VkPhysicalDevice SelectPhysicalDevice();

    VkPhysicalDevice m_PhysicalDeviceHandle;
    QueueFamilyIndices m_QueueIndices;
    VkDevice m_LogicalDeviceHandle;

    std::unique_ptr<VulkanQueue> m_GraphicsQueue;
    std::unique_ptr<VulkanQueue> m_PresentQueue;
    std::unique_ptr<VulkanQueue> m_ComputeQueue;

    const VulkanInstance* m_TmpInstanceHandle;
    std::vector<const char *> m_NeededDeviceExtensions;
};