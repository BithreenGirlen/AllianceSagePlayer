
#include "sdl_clock.h"

CSdlClock::CSdlClock()
{
	Restart();
}

CSdlClock::~CSdlClock()
{

}

float CSdlClock::GetElapsedTime()
{
	Uint64 freq = ::SDL_GetPerformanceFrequency();

	Uint64 nNow = GetNowCounter();
	return (nNow - m_nLastCounter) / (float)freq;
}

void CSdlClock::Restart()
{
	m_nLastCounter = GetNowCounter();
}

Uint64 CSdlClock::GetNowCounter()
{
	return ::SDL_GetPerformanceCounter();
}
