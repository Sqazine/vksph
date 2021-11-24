
#pragma once
#include <memory>
#include "Instance.h"
#include "Device.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
struct WindowCreateInfo
{
    std::string title;
    glm::vec2 extent;
};

namespace VK
{
    class GraphicsContext
    {
    public:
        static void Init(const WindowCreateInfo &windowInfo);
        static void Destroy();

        static Device*  GetDevice() ;
        static Instance* GetInstance() ; 
        static SDL_Window* GetWindow() ;
    private:
        GraphicsContext() {}
        static std::unique_ptr<Instance> m_Instance;
        static std::unique_ptr<Device> m_Device;

        static SDL_Window *m_WindowHandle;
    };
}