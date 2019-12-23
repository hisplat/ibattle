
#pragma once

namespace ib {

class Connector;
class Timer {
public:
    Timer();
    virtual ~Timer();
    virtual bool onTimer(void * arg, int count) = 0;
    virtual void onTimerRemoved() {}
};

class TimerWrapper {
public:
    TimerWrapper(Timer* timer);
    ~TimerWrapper();


    void onRemove();
    bool trigger();
    void schedule();
    long getLeftTime(long now);

    void setArg(void * arg);
    void setInterval(long v);
    long getInterval();
    void setId(int id);
    int getId();

private:
    TimerWrapper() {}
    Timer* mTimer;
    void* mArg;
    int mCount;
    long mInterval;
    long mTimeout;
    int mId;
};
} // namespace ib

