

#include "lock.h"


namespace base {
Lock::Lock()
{
    pthread_mutex_init(&hMutex, NULL);
}

Lock::~Lock()
{
}

void Lock::acquire()
{
    pthread_mutex_lock(&hMutex);
}

void Lock::release()
{
    pthread_mutex_unlock(&hMutex);
}

} // namespace base 

