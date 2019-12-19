
#include "condition.h"
#include "logging.h"

namespace base {

Condition::Condition()
{
    int ret = pthread_mutex_init(&mMutex, NULL);
    LCHECK(ret == 0);

    ret = pthread_cond_init(&mCond, NULL);
    LCHECK(ret == 0);
}

Condition::~Condition()
{
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
}

int Condition::lock()
{
    return pthread_mutex_lock(&mMutex);
}

int Condition::unlock()
{
    return pthread_mutex_unlock(&mMutex);
}

int Condition::wait()
{
    return pthread_cond_wait(&mCond, &mMutex);
}

int Condition::wait(struct timespec * abstime)
{
    return pthread_cond_timedwait(&mCond, &mMutex, abstime);
}

int Condition::signal()
{
    return pthread_cond_signal(&mCond);
}

int Condition::broadcast()
{
    return pthread_cond_broadcast(&mCond);
}

} // namespace base

