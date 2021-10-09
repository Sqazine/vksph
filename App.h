#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <SDL2/SDL.h>
#include "VKContext.h"
struct WindowCreateInfo
{
	std::string title;
	int32_t width;
    int32_t height;
	bool resizeable = false;
};

class App
{
public:
	App(const WindowCreateInfo &info);
	~App();

	void Run();
private:
	void Init();
	void ProcessInput();
	void Update();
	void Draw();

	bool m_IsRunning;

    WindowCreateInfo m_WindowCreateInfo;
	VKContext m_RenderContext;
};
