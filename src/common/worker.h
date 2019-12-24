
#pragma once

#include "base/helper.h"
#include <vector>

namespace ib {

class Poller;
// class Command;
class Worker {
public:
    class Device {
    public:
        Device();
        virtual ~Device();
        DECLARE_CLASS(Device);

        virtual void onDataArrival(int fd) = 0;
        virtual void write(const void * buf, int len) = 0;

        virtual int getFd() = 0;
        virtual void disconnect() { onDisconnected(); }

        virtual int heartbitInterval() { return 0; }
        virtual int heartbitTimeout() { return 0; }

        virtual void dump(std::ostream& o) {}

    public:
        void attach(Worker * w);
        void onData(const void * buf, int len);
        void onDisconnected();
        void onNewConnection(void* arg);
        void onFileChanged(const std::string& path, int mask);

    private:
        Worker * mWorker;
    };

public:
    Worker(Device* device);
    virtual ~Worker(); 
    DECLARE_CLASS(Worker);

    void attach(Poller* poller);
    bool isValid() { return mIsValid; }
    void enable() { mIsValid = true; }
    void disable() { mIsValid = false; }

    // call device
    int getFd();
    void onDataArrival(int fd);

    Device* device() { return mDevice; }
    Poller* poller() { return mPoller; }

    void write(const void * buf, int len);
    bool shouldHeartbit();
    bool hasTimedout();

    long getLastSendTime() { return mLastSendTime; }
    long getLastReceiveTime() { return mLastReceivedTime; }

    virtual void dump(std::ostream& o);
    virtual void onData(const void * buf, int len);
    virtual void doHeartbit() {}

protected:
    Worker(); 

    // // implements from Parser.
    // virtual void onPacketParsed(const void* data, int len); // on receive.
    // virtual void onPostCreateCommand(Command* command) {};
    // virtual void onPacketCreated(const void* data, int len); // on write.
    // virtual void onBeforeCreatePacket(Command * cmd) {};

protected:
    friend class Device;
    // called by device.
    void onDisconnected();
    void onNewConnection(void* arg);
    void onFileChanged(const std::string& path, int mask);

private:
    Poller * mPoller;
    Device* mDevice;
    bool mIsValid;

    long mLastSendTime;
    long mLastReceivedTime;

    // Command* createCommand(const void* data, int len);
};

} // namespace ib 

std::ostream& operator<<(std::ostream& o, ib::Worker& worker);
std::ostream& operator<<(std::ostream& o, ib::Worker* worker);
std::ostream& operator<<(std::ostream& o, ib::Worker::Device* device);

