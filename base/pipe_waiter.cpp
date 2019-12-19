
#include "pipe_waiter.h"

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

namespace base {

PipeWaiter::PipeWaiter()
{
    pipe(fd);
}

PipeWaiter::~PipeWaiter()
{
    close(fd[0]);
    close(fd[1]);
}

void PipeWaiter::wait(int timeoutms)
{
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeoutms / 1000;
    tv.tv_usec = (timeoutms % 1000) * 1000;
    FD_ZERO(&rfds);
    FD_SET(fd[0], &rfds);

    struct timeval* ptv = NULL;
    if (timeoutms > 0) {
        ptv = &tv;
    }
    int ret = select(fd[0] + 1, &rfds, NULL, NULL, ptv);
    if (ret > 0) {
        char buff[2];
        read(fd[0], buff, 1);
    }
}

void PipeWaiter::notify()
{
    write(fd[1], "a", 1);
}

} // namespace base

