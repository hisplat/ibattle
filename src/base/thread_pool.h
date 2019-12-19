
#pragma once

#include "condition.h"
#include <list>

namespace base {

class ThreadPool {
public:
    ThreadPool(int workers);
    ~ThreadPool();

    class Task {
    public:
        virtual ~Task() {};
        virtual void run() = 0;
    };

    void addTask(Task* task);

private:
    static void* routine(void* arg);
    Condition   mCond;

    int  mWorkers;
    bool mQuit;

    std::list<Task*>  mTasks;
};

} // namespace base

