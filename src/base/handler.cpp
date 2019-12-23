

#include "handler.h"
#include "logging.h"
#include "window.h"

#include <stdlib.h>

namespace base {

Handler::Handler(MessageQueue* queue, Callback* callback)
    : mCallback(callback)
    , mQueue(queue)
{
}

void Handler::handleMessage(Message msg)
{
    NFLOG() << "nobody handle message: " << msg;
}

void Handler::dispatchMessage(Message& msg)
{
    if (msg.callback != NULL) {
        msg.callback->onMessage(msg);
    } else {
        if (mCallback != NULL) {
            if (mCallback->handleMessage(msg)) {
                return;
            }
        }
        handleMessage(msg);
    }
}

void Handler::sendMessageDelayed(Message msg, long delayMillis)
{
    if (mQueue != NULL) {
        mQueue->enqueueMessage(msg, delayMillis);
    }
}

void Handler::sendMessage(Message msg)
{
    sendMessageDelayed(msg, 0);
}

} // namespace base
