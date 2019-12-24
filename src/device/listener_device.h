
#pragma once
#include "net_device.h"

namespace ib {
class ListenerDevice : public NetDevice {
public:
    ListenerDevice(int port);
    virtual ~ListenerDevice();
    DECLARE_CLASS(ListenerDevice);

    // implements from Worker::Device
    virtual void onDataArrival(int fd);
    virtual int heartbitInterval() { return 0; }
    virtual int heartbitTimeout() { return 0; }
private:
    ListenerDevice();
};

} // namespace ib

