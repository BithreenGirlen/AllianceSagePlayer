#ifndef SDL_CLOCK_H_
#define SDL_CLOCK_H_

#include <SDL2/SDL_timer.h>

class CSdlClock
{
public:
    CSdlClock();
    ~CSdlClock();

    float GetElapsedTime();
    void Restart();
private:
    Uint64 m_nLastCounter{};

    Uint64 GetNowCounter();
};
#endif // !SDL_CLOCK_H_

