
#pragma once

#include <list>
#include <sstream>

#include "base/lock.h"

namespace ib {
class Worker;
class Command;
class TimerWrapper;
class Timer;
class HeartbitTimer;
class Poller {
public:
    class Executor {
    public:
        Executor() {}
        virtual ~Executor() {}
        virtual void onCommand(Worker* worker, Command* cmd) = 0;
        virtual void onNewConnection(int fromfd, void* arg) {}
        virtual void onDisconnect(Worker* worker) {}
        virtual void onTimeout(Worker* worker) {}
        virtual void onHeartbit(Worker* worker) {}
        virtual void onFileChanged(Worker* worker, const std::string& path, int mask) {}
    };

public:
    Poller(Executor * executor);
    ~Poller();

    void run();
    void onCommand(Worker* worker, Command* cmd);
    void onNewConnection(int fromfd, void* arg);
    void onDisconnect(Worker* worker);
    void onFileChanged(Worker* worker, const std::string& path, int mask);

    void addWorker(Worker* worker);
    void removeWorker(Worker* worker);
    
    void dump(std::ostream& o);
    void stop();

    void checkHeartbit();


public:
    int addTimer(Timer* timer, long interval, void * arg);
    void removeTimer(int timerid);
    void resetTimer(int timerid);

private:
    Poller();
    Executor* mExecutor;
    std::list<Worker*> mWorkers;
    std::list<TimerWrapper*> mTimers;
    bool mWorkersChanged;
    bool mRunning;
    int mTickFd[2];
    int mTimerId;

    base::Lock mTimerLock;

    void init();
    long nextTimerTimeout();
    void checkAndTriggerTimer();
    void clearAllTimer();
    HeartbitTimer * mHeartbitTimer;

    // epoll
    void epollAdd(int fd);
    void epollDel(int fd);
    int mEpfd;
};

} // namespace ib

std::ostream& operator<<(std::ostream& o, ib::Poller* connector);
std::ostream& operator<<(std::ostream& o, ib::Poller& connector);

