
#pragma once

#include <pthread.h>

namespace base {

class Condition {
public:
    Condition();
    ~Condition();

    int lock();
    int unlock();

    int wait();
    int wait(struct timespec * abstime);

    int signal();
    int broadcast();

private:
    pthread_mutex_t mMutex;
    pthread_cond_t  mCond;
};
} // namespace base

