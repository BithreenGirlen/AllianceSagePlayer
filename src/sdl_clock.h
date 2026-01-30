#ifndef SDL_CLOCK_H_
#define SDL_CLOCK_H_

#include <SDL3/SDL_timer.h>

class CSdlClock
{
public:
    CSdlClock();
    ~CSdlClock();

    float getElapsedTime();
    void restart();
private:
    Uint64 m_nLastCounter{};

    Uint64 getNowCounter();
};
#endif // !SDL_CLOCK_H_

