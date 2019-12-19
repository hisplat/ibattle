
#include "unixsocket.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <vector>

#include "logging.h"
#include <sys/ioctl.h>

namespace base {

UnixSocket::UnixSocket()
    : mSock(-1)
    , mSockType(eSocketType_Stream)
{
    memset(&mLocal, 0, sizeof(mLocal));
    memset(&mRemote, 0, sizeof(mRemote));

    mLocal.sun_family = AF_UNIX;
    mRemote.sun_family = AF_UNIX;
}

UnixSocket::UnixSocket(SOCKETTYPE eSocketType)
    : mSock(-1)
    , mSockType(eSocketType)
{
    memset(&mLocal, 0, sizeof(mLocal));
    memset(&mRemote, 0, sizeof(mRemote));

    mLocal.sun_family = AF_UNIX;
    mRemote.sun_family = AF_UNIX;
}

UnixSocket::UnixSocket(SOCKETTYPE eSocketType, int fd)
    : mSock(fd)
    , mSockType(eSocketType)
{
    memset(&mLocal, 0, sizeof(mLocal));
    memset(&mRemote, 0, sizeof(mRemote));
    mLocal.sun_family = AF_UNIX;
    mRemote.sun_family = AF_UNIX;
}

UnixSocket::UnixSocket(const UnixSocket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mLocal, &o.mLocal, sizeof(mLocal));
    UnixSocket * p = (UnixSocket*)&o;
    p->mSock = -1;
}

UnixSocket::UnixSocket(const UnixSocket&& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mLocal, &o.mLocal, sizeof(mLocal));
    UnixSocket * p = (UnixSocket*)&o;
    p->mSock = -1;
}

UnixSocket& UnixSocket::operator=(const UnixSocket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mLocal, &o.mLocal, sizeof(mLocal));
    UnixSocket * p = (UnixSocket *)&o;
    p->mSock = -1;
    return *this;
}

UnixSocket::~UnixSocket()
{
    reset();
}

bool UnixSocket::create(SOCKETTYPE socketType)
{
    errno = 0;
    SOCKETTYPE ost = ((socketType != eSocketType_None) ? socketType : mSockType);

    reset();
    mSockType = ost;
    if (mSockType == eSocketType_Stream) {
        mSock = socket(PF_UNIX, SOCK_STREAM, 0);
    } else if (mSockType == eSocketType_Dgram) {
        mSock = socket(PF_UNIX, SOCK_DGRAM, 0);
    }
    if (mSock == -1) {
        return false;
    }
    return true;
}

int UnixSocket::fd()
{
    return mSock;
}

std::string UnixSocket::getRemoteAddr()
{
    if (mRemote.sun_path[0] == '\0') {
        if (mRemote.sun_path[1] == '\0') {
            return "";
        }
        std::string tmp = mRemote.sun_path + 1;
        tmp = std::string("*") + tmp;
        return tmp;
    }
    return mRemote.sun_path;

}

bool UnixSocket::setRemoteAddr(const std::string& addr)
{
    snprintf(mRemote.sun_path, sizeof(mRemote.sun_path), "%s", addr.c_str());
    if (mRemote.sun_path[0] == '*') {
        mRemote.sun_path[0] = '\0';
    }
    return true;
}

std::string UnixSocket::getLocalAddr()
{
    if (mLocal.sun_path[0] == '\0') {
        if (mLocal.sun_path[1] == '\0') {
            return "";
        }
        std::string tmp = mLocal.sun_path + 1;
        tmp = std::string("*") + tmp;
        return tmp;
    }
    return mLocal.sun_path;
}

bool UnixSocket::setLocalAddr(const std::string& addr)
{
    snprintf(mLocal.sun_path, sizeof(mLocal.sun_path), "%s", addr.c_str());
    if (mLocal.sun_path[0] == '*') {
        mLocal.sun_path[0] = '\0';
    }
    return true;
}

bool UnixSocket::listen()
{
    errno = 0;
    return (::listen(mSock, 10) == 0);
}

bool UnixSocket::bind(const std::string& addr)
{
    if (addr != "") {
        setLocalAddr(addr);
    }
    // int size = offsetof(struct sockaddr_un, sun_path) + strlen(mLocal.sun_path) + 1;
    if (mLocal.sun_path[0] != '\0') {
        if (access(mLocal.sun_path, F_OK | R_OK) == 0) {
            unlink(mLocal.sun_path);
        }
    }
    errno = 0;
    int ret = ::bind(mSock, (struct sockaddr *)&mLocal, sizeof(mLocal));
    return (ret == 0);
}

bool UnixSocket::connect(const std::string& addr)
{
    if (addr != "") {
        setRemoteAddr(addr);
    }

    // int size = offsetof(struct sockaddr_un, sun_path) + strlen(mLocal.sun_path) + 1;
    errno = 0;
    int ret = ::connect(mSock, (struct sockaddr *)&mRemote, sizeof(mRemote));
    return (ret == 0);
}

UnixSocket UnixSocket::accept()
{
    socklen_t len = sizeof(mRemote);
    errno = 0;
    int fd = ::accept(mSock, (struct sockaddr *)&mRemote, &len);

    UnixSocket sock;
    sock.reset();
    sock.mSock = fd;
    sock.mSockType = mSockType;
    memcpy(&sock.mRemote, &mRemote, sizeof(mRemote));
    memcpy(&sock.mLocal, &mLocal, sizeof(mLocal));
    return sock;
}

int UnixSocket::recv(void * buf, size_t len)
{
    int ret = ::read(mSock, (char *)buf, len);
    if (ret <= 0) {
        onDisconnected();
    } else {
        onReadData(buf, ret);
    }
    return ret;
}

int UnixSocket::send(const void * buf, size_t len)
{
    return ::write(mSock, (const char *)buf, len);
}

void UnixSocket::reset()
{
    if (mSock != -1) {
        close(mSock);
    }
    mSock = -1;
    mSockType = eSocketType_Stream;
    memset(&mLocal, 0, sizeof(mLocal));
    memset(&mRemote, 0, sizeof(mRemote));
    mLocal.sun_family = AF_UNIX;
    mRemote.sun_family = AF_UNIX;
}

bool UnixSocket::isValid(void)
{
    return (mSock != -1);
}

const char * UnixSocket::lastErrorMessage()
{
    return strerror(errno);
}

} // namespace base 

