
#include "poller.h"

#include <algorithm>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <termios.h>

#include "worker.h"
#include "timer.h"
#include "base/logging.h"
#include "base/helper.h"

namespace ib {

class HeartbitTimer : public Timer {
public:
    HeartbitTimer(Poller * connector) : mPoller(connector) {}
    virtual ~HeartbitTimer() {}
    virtual bool onTimer(void * arg, int count) {
        mPoller->checkHeartbit();
        return true;
    }
private:
    Poller* mPoller;
};

Poller::Poller()
    : mExecutor(NULL)
    , mWorkersChanged(false)
    , mRunning(false)
    , mTimerId(0)
    , mHeartbitTimer(NULL)
    , mEpfd(-1)
{
    init();
}

Poller::Poller(Poller::Executor * executor)
    : mExecutor(executor)
    , mWorkersChanged(false)
    , mTimerId(0)
    , mHeartbitTimer(NULL)
    , mEpfd(-1)
{
    init();
}

Poller::~Poller()
{
    clearAllTimer();
    close(mTickFd[0]);
    close(mTickFd[1]);
    close(mEpfd);
    delete mHeartbitTimer;
}

void Poller::init()
{
    mEpfd = epoll_create(65535);
    mTickFd[0] = -1;
    mTickFd[1] = -1;
    pipe(mTickFd);
    mTimers.clear();
    mHeartbitTimer = new HeartbitTimer(this);
    addTimer(mHeartbitTimer, 200L, this);

    if (mTickFd[0] >= 0) {
        epollAdd(mTickFd[0]);
    }
}

void Poller::run()
{
    mRunning = true;

    struct epoll_event events[20];

    while (mRunning) {
        long timeout = nextTimerTimeout();
        int nfds = 0;

        if (timeout > 0) {
            nfds = epoll_wait(mEpfd, events, 20, timeout);
        }
        checkAndTriggerTimer();

        for (int i = 0; i < nfds; i++) {
            if (mTickFd[0] >= 0) {
                if (events[i].data.fd == mTickFd[0]) {
                    char buf[2];
                    read(mTickFd[0], buf, 1);
                }
            }
            mWorkersChanged = false;
            for (std::list<Worker*>::iterator it = mWorkers.begin(); it != mWorkers.end(); it++) {
                Worker* w = *it;
                int fd = w->getFd();

                if (events[i].data.fd == fd) {
                    w->onDataArrival(fd);
                }

                if (mWorkersChanged) {
                    break;  // should check again because some worker(s) has been erased from this loop.
                }
            }
        }
    }
    // RUN_HERE();
}

void Poller::stop()
{
    // RUN_HERE();
    mRunning = false;
}

void Poller::addWorker(Worker* worker)
{
    LCHECK(worker != NULL);
    if (worker == NULL) {
        return;
    }

    std::list<Worker*>::iterator it = std::find(mWorkers.begin(), mWorkers.end(), worker);
    if (it == mWorkers.end()) {
        int fd = worker->getFd();
        LCHECK(fd >= 0);
        if (fd >= 0) {
            mWorkers.push_back(worker);
            worker->attach(this);
            epollAdd(fd);
        }
    }
}


void Poller::removeWorker(Worker* worker)
{
    std::list<Worker*>::iterator it = std::find(mWorkers.begin(), mWorkers.end(), worker);
    if (it == mWorkers.end()) {
        return;
    }

    mWorkers.erase(it);
    mWorkersChanged = true;

    int fd = worker->getFd();
    if (fd >= 0) {
        epollDel(fd);
    }
}


void Poller::epollAdd(int fd)
{
    if (fd < 0) {
        return;
    }
    struct epoll_event mEpollEvent;
    mEpollEvent.data.fd = fd;
    mEpollEvent.events = EPOLLIN;
    epoll_ctl(mEpfd, EPOLL_CTL_ADD, fd, &mEpollEvent);
}

void Poller::epollDel(int fd)
{
    if (fd < 0) {
        return;
    }
    epoll_ctl(mEpfd, EPOLL_CTL_DEL, fd, NULL);
}


void Poller::onCommand(Worker* worker, Command* cmd)
{
    if (mExecutor != NULL) {
        mExecutor->onCommand(worker, cmd);
    }
}

void Poller::onNewConnection(Worker * worker, void* arg)
{
    if (mExecutor != NULL) {
        mExecutor->onNewConnection(worker, arg);
    }
}

void Poller::onFileChanged(Worker* worker, const std::string& path, int mask)
{
    if (mExecutor != NULL) {
        mExecutor->onFileChanged(worker, path, mask);
    }
}

void Poller::onDisconnect(Worker* worker)
{
    removeWorker(worker);
    LCHECK(mExecutor != NULL);
    if (mExecutor != NULL) {
        mExecutor->onDisconnect(worker);
    }
}

void Poller::dump(std::ostream& o)
{
    o << "Poller: {"
      << "Pointer: " << (void*)this << ", "
      << "Executor: " << mExecutor << ", "
      << "Workers: [";
    for (std::list<Worker*>::iterator it = mWorkers.begin(); it != mWorkers.end(); it++) {
        Worker * w = *it;
        if (it != mWorkers.begin()) {
            o << ", ";
        }
        o << w;
    }
    o << "]"
      << "}";
}

int Poller::addTimer(Timer* timer, long interval, void * arg)
{
    base::AutoLock l(mTimerLock);
    int id = mTimerId;
    mTimerId++;
    if (mTimerId >= 0x7ffffff0) {
        mTimerId = 0;
    }

    TimerWrapper * tw = new TimerWrapper(timer);
    tw->setInterval(interval);
    tw->schedule();
    tw->setId(id);
    tw->setArg(arg);
    mTimers.push_back(tw);

    // printf("==============Timer.size:%zu\n", mTimers.size());
    // insert sort
    /*
    std::list<TimerWrapper*>::iterator it;
    for (it = mTimers.begin(); it != mTimers.end(); it++) {
        TimerWrapper * tw = *it;
        if (tw->getLeftTime() > interval) {
            mTimers.insert(it, tw);
            break;
        }
    }
    if (it == mTimers.end()) {
        mTimers.push_back(tw);
    }

    printf("dumpTimer\n");
    for (std::list<TimerWrapper*>::iterator itt = mTimers.begin(); itt != mTimers.end(); itt++) {
        printf("Timer:leftTime:%ld\n", (*itt)->getLeftTime());
    }
    */

    char buf[2] = {0};
    if (mTickFd[1] >= 0) {
        write(mTickFd[1], buf, 1);
    }
    return id;
}

void Poller::removeTimer(int timerid)
{
    base::AutoLock l(mTimerLock);
    for (std::list<TimerWrapper*>::iterator it = mTimers.begin(); it != mTimers.end(); /* */) {
        TimerWrapper * tw = *it;
        if (tw->getId() == timerid) {
            tw->onRemove();
            it = mTimers.erase(it);
            delete tw;
        } else {
            it++;
        }
    }
}

void Poller::resetTimer(int timerid)
{
    base::AutoLock l(mTimerLock);
    for (std::list<TimerWrapper*>::iterator it = mTimers.begin(); it != mTimers.end(); /* */) {
        TimerWrapper * tw = *it;
        if (tw->getId() == timerid) {
            tw->schedule();
            break;
        } else {
            it++;
        }
    }
}

long Poller::nextTimerTimeout()
{
    base::AutoLock l(mTimerLock);
    long next = 1000;
    long now = base::helper::mtick();
    for (std::list<TimerWrapper*>::iterator it = mTimers.begin(); it != mTimers.end(); it++) {
        TimerWrapper * timer = *it;
        long left = timer->getLeftTime(now);
        if (left > 0) {
            if (left < next) {
                next = left;
            }
        } else {
            return left;
        }
    }
    return next;
}

void Poller::checkAndTriggerTimer()
{
    base::AutoLock l(mTimerLock);
    long now = base::helper::mtick();
    for (std::list<TimerWrapper*>::iterator it = mTimers.begin(); it != mTimers.end(); /* it++ */) {
        TimerWrapper * tw = *it;
        int left = tw->getLeftTime(now);
        if (left <= 0) {
            bool b = tw->trigger();
            if (!b) {
                it = mTimers.erase(it);
                tw->onRemove();
                delete tw;
                continue;
            }
        }
        it++;
    }
}

void Poller::clearAllTimer()
{
    base::AutoLock l(mTimerLock);
    for (std::list<TimerWrapper*>::iterator it = mTimers.begin(); it != mTimers.end(); it++) {
        TimerWrapper * tw = *it;
        tw->onRemove();
        delete tw;
    }
    NFLOG() << mTimers.size() << " timers cleared.";
}

void Poller::checkHeartbit()
{
    // NFLOG() << "checkHeartbit";
    for (std::list<Worker*>::iterator it = mWorkers.begin(); it != mWorkers.end(); /* */) {
        Worker* w = *it;
        // onTimeout will delete w, so we need recored it_next
        std::list<Worker*>::iterator it_next = (++it);
        if (!w->isValid()) {
            continue;
        }
        if (w->shouldHeartbit()) {
            w->doHeartbit();
        }
        if (mExecutor != NULL) {
            if (w->hasTimedout()) {
                mExecutor->onTimeout(w);
            }
        }
        it = it_next;
    }
}

} // namespace ib

std::ostream& operator<<(std::ostream& o, ib::Poller* connector)
{
    connector->dump(o);
    return o;
}

std::ostream& operator<<(std::ostream& o, ib::Poller& connector)
{
    o << &connector;
    return o;
}


