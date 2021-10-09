#include "App.h"
#include "Timer.h"


App::App(const WindowCreateInfo &info)
	: m_IsRunning(true), m_WindowCreateInfo(info)
{
}

App::~App()
{
	m_RenderContext.Destroy();
	SDL_Quit();
}

void App::Run()
{
	Init();
	while (m_IsRunning)
	{
		Timer::Update();

		ProcessInput();
		Update();
		Draw();
	}
}

void App::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		SDL_Log("failed to init SDL!");

	m_RenderContext.Init(m_WindowCreateInfo);

	Timer::Init();
}

void App::ProcessInput()
{
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_IsRunning = false;
		}
	}
	const uint8_t *keyboardState = SDL_GetKeyboardState(nullptr);
	if (keyboardState[SDL_SCANCODE_ESCAPE])
		m_IsRunning = false;

}

void App::Update()
{
}

void App::Draw()
{
}