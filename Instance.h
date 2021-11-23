#pragma once
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <vector>
namespace VK
{
    class Instance
    {
    public:
        Instance(SDL_Window *window, const std::vector<const char *> &neededValidationLayers);
        ~Instance();

        const VkInstance &GetVKInstanceHandle() const;
        const VkSurfaceKHR &GetVKSurfaceKHRHandle() const;
        const std::vector<const char *> &GetNeededValidationLayers() const;

    private:
        void CreateDebugUtilsMessenger();
        void CreateSurface();

        VkInstance m_InstanceHandle;
        VkDebugUtilsMessengerEXT m_DebugMessengerHandle;
        VkSurfaceKHR m_SurfaceHandle;

        SDL_Window *m_TmpWindowhandle;
        std::vector<const char *> m_NeededValidationLayers;
    };
}