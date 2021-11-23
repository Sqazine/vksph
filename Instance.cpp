#include "Instance.h"
#include "Utils.h"
#include <iostream>
#include <SDL2/SDL_vulkan.h>
namespace VK
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *)
    {
        std::string ms;

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            ms = "VERBOSE";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ms = "ERROR";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ms = "WARNING";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            ms = "INFO";
            break;
        default:
            ms = "UNKNOWN";
            break;
        }

        std::string mt;

        if (messageType == 7)
            mt = "General | Validation | Performance";
        else if (messageType == 6)
            mt = "Validation | Performance";
        else if (messageType == 5)
            mt = "General | Performance";
        else if (messageType == 4)
            mt = "Performance";
        else if (messageType == 3)
            mt = "General | Validation";
        else if (messageType == 2)
            mt = "Validation";
        else if (messageType == 1)
            mt = "General";
        else
            mt = "Unknown";

        printf("[%s: %s]\n%s\n", ms.c_str(), mt.c_str(), pCallbackData->pMessage);

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    Instance::Instance(SDL_Window *window, const std::vector<const char *> &neededValidationLayers)
        : m_TmpWindowhandle(window), m_NeededValidationLayers(neededValidationLayers)
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pApplicationName = nullptr;
        appInfo.pEngineName = nullptr;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext = nullptr;
        instanceInfo.flags = 0;
        instanceInfo.pApplicationInfo = &appInfo;

        uint32_t instanceExtCount;
        std::vector<const char *> requiredInstanceExts;
        SDL_Vulkan_GetInstanceExtensions(m_TmpWindowhandle, &instanceExtCount, nullptr);
        requiredInstanceExts.resize(instanceExtCount);
        SDL_Vulkan_GetInstanceExtensions(m_TmpWindowhandle, &instanceExtCount, requiredInstanceExts.data());
#if _DEBUG
        requiredInstanceExts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        std::vector<VkExtensionProperties> instanceExtensionProps = GetInstanceExtensionProps();

        bool extSatisfied = CheckExtensionSupport(requiredInstanceExts, instanceExtensionProps);

        if (extSatisfied)
        {
            instanceInfo.enabledExtensionCount = requiredInstanceExts.size();
            instanceInfo.ppEnabledExtensionNames = requiredInstanceExts.data();
        }
        else
        {
            instanceInfo.enabledExtensionCount = 0;
            instanceInfo.ppEnabledExtensionNames = nullptr;
        }

#if _DEBUG
        instanceInfo.enabledLayerCount = m_NeededValidationLayers.size();
        instanceInfo.ppEnabledLayerNames = m_NeededValidationLayers.data();
        if (!CheckValidationLayerSupport(m_NeededValidationLayers, GetInstanceLayerProps()))
            std::cout << "Lack of necessary validation layer" << std::endl;
#endif

        VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_InstanceHandle));

        CreateDebugUtilsMessenger();
        CreateSurface();
    }

    Instance::~Instance()
    {
        vkDestroySurfaceKHR(m_InstanceHandle, m_SurfaceHandle, nullptr);
#if _DEBUG
        DestroyDebugUtilsMessengerEXT(m_InstanceHandle, m_DebugMessengerHandle, nullptr);
#endif
        vkDestroyInstance(m_InstanceHandle, nullptr);
    }

    const VkInstance &Instance::GetVKInstanceHandle() const
    {
        return m_InstanceHandle;
    }

    const VkSurfaceKHR &Instance::GetVKSurfaceKHRHandle() const
    {
        return m_SurfaceHandle;
    }

    const std::vector<const char *> &Instance::GetNeededValidationLayers() const
    {
        return m_NeededValidationLayers;
    }

    void Instance::CreateDebugUtilsMessenger()
    {
#if _DEBUG
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(m_InstanceHandle, &createInfo, nullptr, &m_DebugMessengerHandle) != VK_SUCCESS)
            std::cout << "Failed to create debug messenger" << std::endl;
#endif
    }

    void Instance::CreateSurface()
    {
        if (SDL_Vulkan_CreateSurface(m_TmpWindowhandle, m_InstanceHandle, &m_SurfaceHandle) != SDL_TRUE)
            std::cout << "Failed to create vulkan surface:" << SDL_GetError() << std::endl;
    }
}