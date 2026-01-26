
#include "sdl_clock.h"

CSdlClock::CSdlClock()
{
	restart();
}

CSdlClock::~CSdlClock()
{

}

float CSdlClock::getElapsedTime()
{
	Uint64 freq = ::SDL_GetPerformanceFrequency();

	Uint64 nNow = getNowCounter();
	return (nNow - m_nLastCounter) / (float)freq;
}

void CSdlClock::restart()
{
	m_nLastCounter = getNowCounter();
}

Uint64 CSdlClock::getNowCounter()
{
	return ::SDL_GetPerformanceCounter();
}
