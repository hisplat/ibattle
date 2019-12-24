#include "base_device.h"
#include "base/logging.h"

namespace ib {

BaseDevice::BaseDevice()
{
}

BaseDevice::~BaseDevice()
{
}

int BaseDevice::create()
{
    return -1;
}

void BaseDevice::onDataArrival(int fd)
{
    char buffer[2048];
    int ret = read(buffer, sizeof(buffer));
    if (ret <= 0) {
        PERROR() << "read. ret:" << ret;
        onDisconnected();
    } else {
        onData(buffer, ret);
    }
}

} // namespace ib

