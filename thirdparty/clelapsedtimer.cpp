#include "clelapsedtimer.h"
#include <chrono>

class CLElapsedTimerPrivate
{
public:
    unsigned long long time()
    {
        std::chrono::time_point<std::chrono::system_clock> timestamp =
            std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>
               (timestamp.time_since_epoch()).count();
    }
public:
    unsigned long long upretimer;
};

CLElapsedTimer::CLElapsedTimer():
    d_ptr(new CLElapsedTimerPrivate )
{

}

CLElapsedTimer::~CLElapsedTimer()
{
    if (d_ptr)
        delete d_ptr;
}

void CLElapsedTimer::start()
{
    restart();
}

void CLElapsedTimer::restart()
{
    d_ptr->upretimer = d_ptr->time();
}

unsigned long long CLElapsedTimer::nsecsElapsed()
{
    return d_ptr->time() - d_ptr->upretimer;
}

unsigned long long CLElapsedTimer::usElapsed()
{
    return nsecsElapsed() / 1000.0;
}

unsigned long long CLElapsedTimer::msElapsed()
{
    return usElapsed() / 1000.0;
}



