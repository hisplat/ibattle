
#include "message_queue.h"
#include "logging.h"

#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace base {

MessageQueue::MessageQueue()
{
    if (pipe(pipefd) != 0) {
        pipefd[0] = -1;
        pipefd[1] = -1;
        NFLOG() << "create pipe failed.";
    }
}

MessageQueue::~MessageQueue()
{
    if (pipefd[0] != -1)
        close(pipefd[0]);
    if (pipefd[1] != -1)
        close(pipefd[1]);
}


long MessageQueue::tick()
{
    struct timespec ts;
    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long n = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return n;
}

long long MessageQueue::ntick()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (long long)(t.tv_sec)*1000000000LL + t.tv_nsec;
}


bool MessageQueue::enqueueMessage(Message msg, long delayMillis)
{
    AutoLock l(mLock);

    long when = tick() + delayMillis;
    msg.when = when;

    std::list<Message>::iterator it = mQueue.end();
    for (it = mQueue.begin(); it != mQueue.end(); it++) {
        Message & m = *it;
        if (m.when > when) {
            break;
        }
    }
    if (it == mQueue.end()) {
        mQueue.push_back(msg);
    } else {
        mQueue.insert(it, msg);
    }

    if (pipefd[1] != -1) {
        int tmp = 0;
        ::write(pipefd[1], &tmp, 1);
    }

    return true;
}

void MessageQueue::removeMessages(int what, Message::Callback * callback)
{
    AutoLock l(mLock);
    if (!mQueue.empty()) {
        for (std::list<Message>::iterator it = mQueue.begin(); it != mQueue.end(); /* nothing */) {
            Message& m = *it;
            if (m.what == what && (callback == NULL || m.callback == callback)) {
                it = mQueue.erase(it);
            } else {
                it++;
            }
        }
    }
}

Message MessageQueue::next()
{
    // NFLOG() << this;
    // ScopeTrace(Queue);
    long timeout = 0;
    while (true) {
        long now = tick();
        long to = (timeout == 0) ? 0 : timeout - now;

        if (to > 0) {
            fd_set rfds;
            struct timeval tv;

            FD_ZERO(&rfds);
            if (pipefd[0] != -1) {
                FD_SET(pipefd[0], &rfds);
            }

            tv.tv_sec = to / 1000;
            tv.tv_usec = (to % 1000) * 1000;

            int ret = select(pipefd[0] + 1, &rfds, NULL, NULL, &tv);
            if (ret < 0) {
                NFLOG() << "ret < 0 errno = " << errno << ": " << strerror(errno);
                continue;
            }
            if (ret > 0 && FD_ISSET(pipefd[0], &rfds)) {
                char buffer[1024];
                ::read(pipefd[0], buffer, sizeof(buffer));
            }
        }
        {
            AutoLock l(mLock);
            timeout = 0;
            std::list<Message>::iterator it = mQueue.end();
            if (!mQueue.empty()) {
                for (it = mQueue.begin(); it != mQueue.end(); it++) {
                    Message& m = *it;
                    if (m.when <= now) {
                        Message msg = m;
                        mQueue.erase(it);
                        return msg;
                    }
                    if (timeout == 0 || m.when < timeout) {
                        timeout = m.when;
                        // NFLOG() << "timeout = " << timeout << m;
                        // NFLOG() << "now = " << tick();
                    }
                }
            }
            if (timeout == 0) {
                timeout = tick() + 20;
            }

            if (pipefd[0] == -1 && timeout > tick() + 20) {
                timeout = tick() + 20;
            }
        }
    }

    // Message msg = mQueue.front();
    // mQueue.pop_front();
    // return msg;
}

void MessageQueue::dump(std::ostream& o)
{
    o << "MessageQueue { " << std::endl;
    for (std::list<Message>::iterator it = mQueue.begin(); it != mQueue.end(); it++) {
        o << *it << std::endl;
    }
    o << "}";
}

} // namespace base

std::ostream& operator<<(std::ostream& o, base::MessageQueue& queue)
{
    queue.dump(o);
    return o;
}

std::ostream& operator<<(std::ostream& o, base::MessageQueue* queue)
{
    queue->dump(o);
    return o;
}


