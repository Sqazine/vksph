#include "Timer.h"
#include <SDL2/SDL.h>
uint32_t Timer::m_StartTick = 0;
	uint32_t Timer::m_CurTicks = 0;
	float Timer::m_DeltaTime = 0.0f;

	Timer::Timer()
	{
	}

	void Timer::Init()
	{
		m_StartTick = SDL_GetTicks();
	}

	void Timer::Update(uint32_t lockFrame)
	{
		if (lockFrame != 0)
			while (!SDL_TICKS_PASSED(SDL_GetTicks(), m_CurTicks + (1.0f / lockFrame) * 1000))
				;

		m_DeltaTime = (SDL_GetTicks() - m_CurTicks) / 1000.0f;
		m_CurTicks = SDL_GetTicks();
	}

	float Timer::GetDeltaTime()
	{
		return m_DeltaTime;
	}
