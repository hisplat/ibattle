

#pragma once

#include <pthread.h>
namespace base {

class Lock
{
public:
    Lock();
    ~Lock();

    void acquire();
    void release();

private:
    pthread_mutex_t hMutex;
};

class AutoLock {
public:
    AutoLock(Lock& lock, bool condition = true) : mCondition(condition), lock_(lock) { if (mCondition) lock_.acquire(); }
    ~AutoLock() { if (mCondition) lock_.release(); }
private:
    bool mCondition;
    Lock& lock_;
};

} // namespace base;

