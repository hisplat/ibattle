
#include "stdin_device.h"
#include "base/buffer.h"
#include "base/logging.h"

namespace ib {

StdinDevice::StdinDevice()
    : Worker::Device()
{
}

StdinDevice::~StdinDevice()
{
}


void StdinDevice::onDataArrival(int fd)
{
    char buffer[2048];
    int ret = read(fd, buffer, sizeof(buffer) - 1);
    buffer[ret] = '\0';
    while (ret > 0 && (buffer[ret - 1] == '\r' || buffer[ret - 1] == '\n')) {
        ret--;
        buffer[ret] = '\0';
    }
    onData(buffer, strlen(buffer) + 1);
}

void StdinDevice::write(const void * buf, int len)
{
    // base::Buffer buffer(buf, len);
    // QUIET() << buffer;
    printf("<gate>: %s\n", (const char *)buf);
}

} // namespace ib

