#include "inotify_device.h"
#include "base/logging.h"
#include <sys/inotify.h>
#include <unistd.h>

namespace ib {

InotifyDevice::InotifyDevice()
{
    mFd = inotify_init();
    PERROR_IF(mFd == -1) << "inotify init fail.";
}

InotifyDevice::~InotifyDevice()
{
    close(mFd);
}

void InotifyDevice::watch(const std::string& path)
{
    int ret = inotify_add_watch(mFd, path.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_ALL_EVENTS);
    PERROR_IF(ret < 0) << "watch path failed: " << path;
    mPath = path;
}

void InotifyDevice::onDataArrival(int fd)
{
    char buffer[2048];
    int ret = read(mFd, buffer, sizeof(buffer));
    if (ret < (int)sizeof(struct inotify_event)) {
        onDisconnected();
    } else {
        int offset = 0;
        while (ret > (int)sizeof(struct inotify_event)) {
            struct inotify_event * event = (struct inotify_event*)(buffer + offset);
            if (event->len > 0) {
                if (event->mask & IN_CREATE) {
                    onFileChanged(mPath, IN_CREATE);
                } else if (event->mask & IN_DELETE) {
                    onFileChanged(mPath, IN_DELETE);
                } else if (event->mask & IN_MODIFY) {
                    onFileChanged(mPath, IN_MODIFY);
                } else {
                    onFileChanged(mPath, -1);
                }
            }
            int size = sizeof(struct inotify_event) + event->len;
            ret -= size;
            offset += size;
        }
    }
}

} // namespace ib

