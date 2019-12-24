
#pragma once
#include "base_device.h"
#include "unixsocket.h"

namespace ib {

class UnixDevice : public BaseDevice {
public:
    UnixDevice();
    UnixDevice(const UnixSocket& s);
    UnixDevice(const UnixSocket* s);
    virtual ~UnixDevice();
    DECLARE_CLASS(UnixDevice);

    // implements from BaseDevice
    virtual int read(char * buf, int len);

    // implements from Worker::Device
    virtual void write(const void * buf, int len);
    virtual int getFd();
    virtual int heartbitInterval() { return 0; }
    virtual int heartbitTimeout() { return 0; }

    virtual void disconnect();

protected:
    UnixSocket mSocket;
};

} // namespace ib

