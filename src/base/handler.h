
#pragma once


#include "message.h"
#include "message_queue.h"

namespace base {

class Handler {
public:
    class Callback {
    public:
        virtual bool handleMessage(Message msg) = 0;
        virtual ~Callback() {}
    };


    Handler(MessageQueue* queue, Callback* callback = NULL);
    virtual ~Handler() {}

    void sendMessage(Message msg);
    void sendMessageDelayed(Message msg, long delayMillis);

public:
    void dispatchMessage(Message& msg);

    virtual void handleMessage(Message msg);


private:
    Callback* mCallback;
    MessageQueue*   mQueue;
};

} // namespace base;

