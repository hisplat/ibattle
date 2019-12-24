
#pragma once
#include "unix_device.h"
#include <string>

namespace ib {
class UnixListenerDevice : public UnixDevice {
public:
    UnixListenerDevice() = delete;
    UnixListenerDevice(const std::string& addr);
    virtual ~UnixListenerDevice();
    DECLARE_CLASS(UnixListenerDevice);

    // implements from Worker::Device
    virtual void onDataArrival(int fd);
};

} // namespace ib

