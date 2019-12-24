
#pragma once
#include "worker.h"
#include <string>

namespace ib {

class InotifyDevice : public Worker::Device {
public:
    InotifyDevice();
    virtual ~InotifyDevice();
    DECLARE_CLASS(InotifyDevice);

    void watch(const std::string& path);
    std::string path() { return mPath; }

    virtual void onDataArrival(int fd);
    virtual void write(const void * buf, int len) {}
    virtual int getFd() { return mFd; }

private:
    int mFd;
    std::string mPath;

};

} // namespace ib

