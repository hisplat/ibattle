#include <sys/stat.h>
#include <unistd.h>

#include "unix_listener_device.h"
#include "base/logging.h"

namespace ib {

UnixListenerDevice::UnixListenerDevice(const std::string& addr)
    : UnixDevice()
{
    mSocket.create();
    bool b = mSocket.bind(addr);
    LCHECK(b == true) << "bind addr " << addr << " failed.";
    mSocket.listen();
    int ret = chmod(addr.c_str(), 00777);
    LCHECK(ret == 0) << "chmod addr " << addr << " to 0777 failed. ret = " << ret;
    NFLOG() << "bind on " << addr;
    NFLOG() << "listen on " << mSocket.getLocalAddr();
}

UnixListenerDevice::~UnixListenerDevice()
{
}

void UnixListenerDevice::onDataArrival(int fd)
{
    LCHECK(fd == mSocket.fd());

    UnixSocket s = mSocket.accept();
    RUN_HERE() << "accepted fd " << s.fd() << " from " << s.getRemoteAddr();
    onNewConnection(getFd(), &s);
}

} // namespace ib


