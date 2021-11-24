#include "GraphicsContext.h"

namespace VK
{
    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::unique_ptr<Instance> GraphicsContext::m_Instance = nullptr;
    std::unique_ptr<Device> GraphicsContext::m_Device = nullptr;

    SDL_Window *GraphicsContext::m_WindowHandle = nullptr;

    void GraphicsContext::Init(const WindowCreateInfo &windowInfo)
    {
        uint32_t windowFlag = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;

        m_WindowHandle = SDL_CreateWindow(windowInfo.title.c_str(),
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          windowInfo.extent.x,
                                          windowInfo.extent.y,
                                          windowFlag);

        int flag = SDL_Vulkan_LoadLibrary(nullptr);
        if (flag != 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Not Support vulkan:%s\n", SDL_GetError());
            exit(1);
        }

        void *instanceProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
        if (!instanceProcAddr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "SDL_Vulkan_GetVkGetInstanceProcAddr(): %s\n",
                         SDL_GetError());
            exit(1);
        }

        m_Instance = std::make_unique<VK::Instance>(m_WindowHandle, validationLayers);
        m_Device = std::make_unique<VK::Device>(deviceExtensions);
    }
    void GraphicsContext::Destroy()
    {
        SDL_Vulkan_UnloadLibrary();
        SDL_DestroyWindow(m_WindowHandle);
    }

    Device *GraphicsContext::GetDevice()
    {
        return m_Device.get();
    }

    Instance *GraphicsContext::GetInstance()
    {
        return m_Instance.get();
    }

    SDL_Window *GraphicsContext::GetWindow()
    {
        return m_WindowHandle;
    }
}