
#include "thread_pool.h"
#include "logging.h"

namespace base {

void* ThreadPool::routine(void* arg)
{
    ThreadPool * pool = (ThreadPool*)arg;

    char name[100];
    snprintf(name, sizeof(name), "ThreadPool-%d", logging::currentThreadId());
    logging::setThreadName(name);

    while (true) {
        pool->mCond.lock();
        if (pool->mQuit) {
            break;
        }

        if (pool->mTasks.empty()) {
            pool->mCond.wait();
            pool->mCond.unlock();
            continue;
        }

        Task* task = pool->mTasks.front();
        pool->mTasks.pop_front();

        pool->mCond.unlock();

        NFLOG() << "Running task " << task << ".";
        task->run();

        NFLOG() << "Destroy task " << task << ".";
        delete task;
    }

    pool->mWorkers--;
    if (pool->mWorkers == 0) {
        pool->mCond.signal();
    }
    pool->mCond.unlock();
    NFLOG() << "worker thread " << (void *)pthread_self() << " destroyed.";
    return NULL;

}

ThreadPool::ThreadPool(int tasks)
    : mWorkers(tasks)
    , mQuit(false)
{
    LCHECK(tasks > 0);

    for (int i = 0; i < tasks; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, routine, this);
        pthread_detach(thread);
        NFLOG() << "worker thread " << (void *)thread << " created.";
    }
}

ThreadPool::~ThreadPool()
{
    while (true) {
        mCond.lock();
        bool empty = mTasks.empty();
        mCond.unlock();
        if (empty) {
            break;
        }
    }
    mCond.lock();
    mQuit = true;
    mCond.broadcast();
    mCond.wait();
    mCond.unlock();
}

void ThreadPool::addTask(ThreadPool::Task* task)
{
    mCond.lock();
    mTasks.push_back(task);
    mCond.signal();
    mCond.unlock();
    NFLOG() << "Task " << task << " added";
}

} // namespace base

#ifdef __THREAD_POOL_UNIT_TEST__
class mTask : public base::ThreadPool::Task {
public:
    mTask(int id) : mId(id) {}
    virtual void run() {
        NFLOG() << "running task " << mId;
    }
    int mId;
};

int main()
{

    base::ThreadPool * pool = new base::ThreadPool(5);
    for (int i = 0; i < 0; i++) {
        mTask * t = new mTask(i);
        pool->addTask(t);
    }
    delete pool;

    return 0;
}
#endif

