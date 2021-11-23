#include "Device.h"
#include <iostream>
namespace VK
{
    Device::Device(const Instance *instance, std::vector<const char *> neededDeviceExtensions)
        : m_TmpInstanceHandle(instance), m_NeededDeviceExtensions(neededDeviceExtensions)
    {
        m_PhysicalDeviceHandle = SelectPhysicalDevice();

        m_QueueIndices = FindQueueFamilies(m_PhysicalDeviceHandle, m_TmpInstanceHandle->GetVKSurfaceKHRHandle());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        if (m_QueueIndices.IsSameFamily())
        {
            const float queuePriorities[3]{1, 1, 1};
            VkDeviceQueueCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.pQueuePriorities = queuePriorities;
            info.queueCount = 3;
            info.queueFamilyIndex = m_QueueIndices.graphicsFamily.value();
            queueCreateInfos.emplace_back(info);
        }
        else
        {
            const float queuePriorities = 1.0f;
            std::vector<uint32_t> uniIndices = {m_QueueIndices.graphicsFamily.value(), m_QueueIndices.presentFamily.value(), m_QueueIndices.computeFamily.value()};
            for (auto idx : uniIndices)
            {
                VkDeviceQueueCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;
                info.pQueuePriorities = &queuePriorities;
                info.queueCount = 1;
                info.queueFamilyIndex = idx;
                queueCreateInfos.emplace_back(info);
            }
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.enabledExtensionCount = m_NeededDeviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = m_NeededDeviceExtensions.data();
#if _DEBUG
        deviceCreateInfo.enabledLayerCount = m_TmpInstanceHandle->GetNeededValidationLayers().size();
        deviceCreateInfo.ppEnabledLayerNames = m_TmpInstanceHandle->GetNeededValidationLayers().data();
#endif
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        VK_CHECK(vkCreateDevice(m_PhysicalDeviceHandle, &deviceCreateInfo, nullptr, &m_LogicalDeviceHandle));

        m_GraphicsQueue = std::make_unique<Queue>(this, m_QueueIndices.graphicsFamily.value(), 0);
        m_PresentQueue = std::make_unique<Queue>(this, m_QueueIndices.presentFamily.value(), 0);
        m_ComputeQueue = std::make_unique<Queue>(this, m_QueueIndices.computeFamily.value(), 0);
    }
    Device::~Device()
    {
        vkDestroyDevice(m_LogicalDeviceHandle, nullptr);
    }

    void Device::WaitIdle()
    {
        vkDeviceWaitIdle(m_LogicalDeviceHandle);
    }

    const VkDevice &Device::GetLogicalDeviceHandle() const
    {
        return m_LogicalDeviceHandle;
    }

    const VkPhysicalDevice &Device::GetPhysicalDeviceHandle() const
    {
        return m_PhysicalDeviceHandle;
    }

    const Queue *Device::GetGraphicsQueue() const
    {
        return m_GraphicsQueue.get();
    }
    const Queue *Device::GetPresentQueue() const
    {
        return m_PresentQueue.get();
    }
    const Queue *Device::GetComputeQueue() const
    {
        return m_ComputeQueue.get();
    }

    QueueFamilyIndices Device::GetQueueIndices() const
    {
        return m_QueueIndices;
    }

    uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties phyDevMemProps = GetPhysicalDeviceMemoryProps(m_PhysicalDeviceHandle);

        for (uint32_t i = 0; i < phyDevMemProps.memoryTypeCount; ++i)
            if ((typeFilter & (1 << i)) && (phyDevMemProps.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        std::cout << "Failed to find suitable memory type!" << std::endl;
        return -1;
    }

    VkPhysicalDevice Device::SelectPhysicalDevice()
    {
        VkPhysicalDevice result;

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_TmpInstanceHandle->GetVKInstanceHandle(), &physicalDeviceCount, nullptr);
        if (physicalDeviceCount == 0)
            std::cout << "No GPU support vulkan" << std::endl;
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_TmpInstanceHandle->GetVKInstanceHandle(), &physicalDeviceCount, physicalDevices.data());

        for (auto phyDev : physicalDevices)
        {
            std::vector<VkExtensionProperties> phyDevExtensions = GetPhysicalDeviceExtensionProps(phyDev);
            VkPhysicalDeviceFeatures features = GetPhysicalDeviceFeatures(phyDev);

            bool extSatisfied = CheckExtensionSupport(m_NeededDeviceExtensions, phyDevExtensions);

            if (extSatisfied && FindQueueFamilies(phyDev, m_TmpInstanceHandle->GetVKSurfaceKHRHandle()).IsComplete())
                result = phyDev;
            VkPhysicalDeviceProperties props = GetPhysicalDeviceProps(result);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                break;
        }

        VkPhysicalDeviceProperties props = GetPhysicalDeviceProps(result);
        std::vector<VkExtensionProperties> phyDevExtensions = GetPhysicalDeviceExtensionProps(result);

        std::cout << "[INFO]Selected device name: " << props.deviceName << std::endl
                  << "[INFO]Selected device type: ";
        switch (props.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_OTHER";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_CPU";
            break;
        default:;
        }
        std::cout << " (" << props.deviceType << ")" << std::endl
                  << "[INFO]Selected device driver version: "
                  << VK_VERSION_MAJOR(props.driverVersion) << "."
                  << VK_VERSION_MINOR(props.driverVersion) << "."
                  << VK_VERSION_PATCH(props.driverVersion) << std::endl
                  << "[INFO]Selected device vulkan api version: "
                  << VK_VERSION_MAJOR(props.apiVersion) << "."
                  << VK_VERSION_MINOR(props.apiVersion) << "."
                  << VK_VERSION_PATCH(props.apiVersion) << std::endl;

        std::cout << "[INFO]Selected device available extensions:" << std::endl;
        for (const auto &extension : phyDevExtensions)
        {
            std::cout << "[INFO]     name: " << extension.extensionName << " spec_ver: "
                      << VK_VERSION_MAJOR(extension.specVersion) << "."
                      << VK_VERSION_MINOR(extension.specVersion) << "."
                      << VK_VERSION_PATCH(extension.specVersion) << std::endl;
        }

        return result;
    }

}