
#pragma once
#include "common/worker.h"
#include <unistd.h>


namespace ib {

class StdinDevice: public Worker::Device {
public:
    StdinDevice();
    virtual ~StdinDevice();
    DECLARE_CLASS(StdinDevice);

    virtual void onDataArrival(int fd);
    virtual void write(const void * buf, int len);

    virtual int getFd() { return STDIN_FILENO; }
    virtual void dump(std::ostream& o) {}
};

} // namespace ib

