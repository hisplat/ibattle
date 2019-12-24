
#pragma once
#include "common/worker.h"


namespace ib {

// BaseDevice does not contain any fd.
// So 'read/write' should be implemented by derived classes.
class BaseDevice : public Worker::Device {
public:
    BaseDevice();
    virtual ~BaseDevice();
    DECLARE_CLASS(BaseDevice);

    virtual int create();
    virtual int read(char * buf, int len) = 0;

    // implements from Worker::Device
    virtual void onDataArrival(int fd);
    // virtual void write(const void * buf, int len);
    // virtual int getFd();

};

} // namespace ib

