
#pragma once

#include <list>
#include "message.h"
#include "lock.h"

namespace base {

class Lock;
class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();
    bool enqueueMessage(Message msg, long delayMillis);
    Message next();

    void removeMessages(int what, Message::Callback * callback = NULL);

    bool isEmpty() { return mQueue.empty(); }
    long tick();
    long long ntick();

    void dump(std::ostream& o);
private:
    std::list<Message> mQueue;
    Lock    mLock;
    int pipefd[2];
};

} // namespace base

std::ostream& operator<<(std::ostream& o, base::MessageQueue& queue);
std::ostream& operator<<(std::ostream& o, base::MessageQueue* queue);

