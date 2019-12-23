
#include "timer.h"
#include "helper.h"
#include <stdlib.h>

namespace ib {
Timer::Timer()
{
}

Timer::~Timer()
{
}

TimerWrapper::TimerWrapper(Timer * timer)
    : mTimer(timer)
    , mArg(NULL)
    , mCount(0)
    , mInterval(0)
    , mTimeout(0)
    , mId(0)
{
}

TimerWrapper::~TimerWrapper()
{
}

void TimerWrapper::setArg(void* arg)
{
    mArg = arg;
}

void TimerWrapper::setInterval(long v)
{
    mInterval = v;
}

long TimerWrapper::getInterval()
{
    return mInterval;
}

void TimerWrapper::schedule()
{
    long now = base::helper::mtick();
    mTimeout = now + mInterval;
}

long TimerWrapper::getLeftTime(long now)
{
    // long now = helper::mtick();
    return mTimeout - now;
}

bool TimerWrapper::trigger()
{
    mCount++;
    bool b = mTimer->onTimer(mArg, mCount);
    schedule();
    return b;
}

void TimerWrapper::onRemove()
{
    mTimer->onTimerRemoved();
}

void TimerWrapper::setId(int id)
{
    mId = id;
}

int TimerWrapper::getId()
{
    return mId;
}

} // namespace ib 

