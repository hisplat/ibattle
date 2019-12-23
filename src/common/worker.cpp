
#include <stdio.h>

#include "worker.h"
#include "poller.h"
#include "base/helper.h"
#include "base/logging.h"

namespace ib {

Worker::Worker()
    : mPoller(NULL)
    , mDevice(NULL)
    , mIsValid(true)
    , mLastSendTime(0)
{
    mLastReceivedTime = base::helper::mtick();
}

Worker::Worker(Worker::Device* device)
    : mPoller(NULL)
    , mDevice(device)
    , mIsValid(true)
    , mLastSendTime(0)
{
    mLastReceivedTime = base::helper::mtick();
    device->attach(this);
}


Worker::~Worker()
{
}

void Worker::attach(Poller* poller)
{
    mPoller = poller;
}

void Worker::onData(const void* buf, int len)
{
    mLastReceivedTime = base::helper::mtick();
    // for (int ret = parse(buf, len); ret >= 0; ret = parse(NULL, 0)) {
    //     // packet data will send to onPacketParsed().
    // }
}

void Worker::onDisconnected()
{
    disable();
    mPoller->onDisconnect(this);
}

void Worker::onNewConnection(int fromfd, void* arg)
{
    mLastReceivedTime = base::helper::mtick();
    mPoller->onNewConnection(fromfd, arg);
}

void Worker::onFileChanged(const std::string& path, int mask)
{
    mPoller->onFileChanged(this, path, mask);
}

int Worker::getFd()
{
    if (mDevice != NULL) {
        return mDevice->getFd();
    }
    return -1;
}

void Worker::write(const void * buf, int len)
{
    mLastSendTime = base::helper::mtick();
    if (mDevice != NULL) {
        mDevice->write(buf, len);
    }
}

void Worker::onDataArrival(int fd)
{
    if (mDevice != NULL) {
        mDevice->onDataArrival(fd);
    }
}

void Worker::dump(std::ostream& o)
{
}

bool Worker::shouldHeartbit()
{
    if (mDevice == NULL) {
        return false;
    }
    long interval = (long)mDevice->heartbitInterval();
    if (interval <= 0) {
        return false;
    }
    long now = base::helper::mtick();
    return (now - mLastSendTime > interval);
}

bool Worker::hasTimedout()
{
    if (mDevice == NULL) {
        return false;
    }
    long timeout = (long)mDevice->heartbitTimeout();
    if (timeout <= 0) {
        return false;
    }
    long now = base::helper::mtick();
    bool isTimeout = (now - mLastReceivedTime > timeout);
    if (isTimeout) {
        mLastReceivedTime = base::helper::mtick();
    }
    return isTimeout;
}

//----------------------------- Worker::Device ----------------------------------
Worker::Device::Device()
    : mWorker(NULL)
{
}

Worker::Device::~Device()
{
}

void Worker::Device::attach(Worker* w)
{
    mWorker = w;
}

void Worker::Device::onData(const void* buf, int len)
{
    if (mWorker != NULL) {
        mWorker->onData(buf, len);
    }
}

void Worker::Device::onDisconnected()
{
    if (mWorker != NULL) {
        mWorker->onDisconnected();
    }
}

void Worker::Device::onNewConnection(int fromfd, void* arg)
{
    if (mWorker != NULL) {
        mWorker->onNewConnection(fromfd, arg);
    }
}

void Worker::Device::onFileChanged(const std::string& path, int mask)
{
    if (mWorker != NULL) {
        mWorker->onFileChanged(path, mask);
    }
}

} // namespace ib 

std::ostream& operator<<(std::ostream& o, ib::Worker* worker)
{
    if (worker == NULL) {
        o << "Worker : {Pointer: " << (void*)worker << "}";
    } else {
        o << "Worker: {"
          << "Pointer: " << (void*)worker << ", "
          << "Class: " << worker->getClassName() << ", "
          << "LastSendTime: " << worker->getLastSendTime() << ", "
          << "LastReceiveTime: " << worker->getLastReceiveTime() << ", "
          << "Poller: " << (void*)worker->poller() << ", ";
        worker->dump(o);
        o << worker->device()
          << "}";
    }
    return o;
}

std::ostream& operator<<(std::ostream& o, ib::Worker& worker)
{
    o << &worker;
    return o;
}

std::ostream& operator<<(std::ostream& o, ib::Worker::Device* device)
{
    if (device == NULL) {
        o << "Device: {Pointer: " << (void*)device << "}";
    } else {
        o << "Device: {"
          << "Pointer: " << (void*)device << ", "
          << "Class: " << device->getClassName() << ", "
          << "Fd: " << device->getFd() << ", ";
        device->dump(o);
        o << "}";
    }
    return o;
}

