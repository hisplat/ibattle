
#include "listener_device.h"
#include "base/logging.h"

namespace ib {

ListenerDevice::ListenerDevice()
{
}

ListenerDevice::ListenerDevice(int port)
    : NetDevice()
{
    mSocket.create();
    bool b = mSocket.bind(port);
    LCHECK(b == true) << "bind port " << port << " failed.";
    mSocket.listen();
    NFLOG() << "listen on " << mSocket.getHostIP() << ":" << mSocket.getHostPort();
}

ListenerDevice::~ListenerDevice()
{
}

void ListenerDevice::onDataArrival(int fd)
{
    LCHECK(fd == mSocket.fd());

    base::Socket s = mSocket.accept();
    RUN_HERE() << "accepted = " << s.fd();
    onNewConnection(&s);
}

} // namespace ib


