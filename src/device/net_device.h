
#pragma once
#include "base_device.h"
#include "base/socket.h"

namespace ib {

class NetDevice : public BaseDevice {
public:
    NetDevice();
    NetDevice(const base::Socket& s);
    NetDevice(const base::Socket* s);
    virtual ~NetDevice();
    DECLARE_CLASS(NetDevice);

    // virtual int create();

    // implements from BaseDevice
    virtual int read(char * buf, int len);

    // implements from Worker::Device
    virtual void write(const void * buf, int len);
    virtual int getFd();
    virtual int heartbitInterval();
    virtual int heartbitTimeout();

    virtual void disconnect();

protected:
    base::Socket mSocket;
};

} // namespace ib

