
#include "unix_device.h"
#include "base::helper.h"
#include <utility>

namespace ib {

UnixDevice::UnixDevice()
    : mSocket()
{
}

UnixDevice::UnixDevice(const UnixSocket& s)
    : mSocket(s)
{
}

UnixDevice::UnixDevice(const UnixSocket* s)
    : mSocket(std::move(*s))
{
}

UnixDevice::~UnixDevice()
{
    mSocket.reset();
}

int UnixDevice::read(char * buf, int len)
{
    return mSocket.recv(buf, len);
}

void UnixDevice::write(const void * buf, int len)
{
    mSocket.send(buf, len);
}

int UnixDevice::getFd()
{
    return mSocket.fd();
}

void UnixDevice::disconnect()
{
    mSocket.reset();
    Worker::Device::disconnect();
}

} // namespace ib

