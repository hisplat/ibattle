#include "net_device.h"
#include "base/helper.h"
#include <utility>

namespace ib {

NetDevice::NetDevice()
    : mSocket()
{
}

NetDevice::NetDevice(const base::Socket& s)
    : mSocket(s)
{
}

NetDevice::NetDevice(const base::Socket* s)
    : mSocket(std::move(*s))
{
}

NetDevice::~NetDevice()
{
    mSocket.reset();
}

int NetDevice::read(char * buf, int len)
{
    return mSocket.recv(buf, len);
}

void NetDevice::write(const void * buf, int len)
{
    mSocket.send(buf, len);
}

int NetDevice::getFd()
{
    return mSocket.fd();
}

int NetDevice::heartbitInterval()
{
    return 10 * 1000;
}

int NetDevice::heartbitTimeout()
{
    return 30 * 1000;
}

void NetDevice::disconnect()
{
    mSocket.reset();
    Worker::Device::disconnect();
}

} // namespace ib

