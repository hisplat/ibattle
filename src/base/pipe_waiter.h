
#pragma once

namespace base {

class PipeWaiter {
public:
    PipeWaiter();
    ~PipeWaiter();

    void wait(int timeoutms = 0);
    void notify();
private:
    int fd[2];
};
} // namespace base

