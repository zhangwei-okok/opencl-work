#ifndef CLELAPSEDTIMER_H
#define CLELAPSEDTIMER_H

class CLElapsedTimerPrivate;
class CLElapsedTimer
{
public:
    CLElapsedTimer();
    virtual  ~CLElapsedTimer();
    void start();
    void restart();
    unsigned long long nsecsElapsed();
    unsigned long long usElapsed();
    unsigned long long msElapsed();
private:
    CLElapsedTimerPrivate* d_ptr;
};

#endif
