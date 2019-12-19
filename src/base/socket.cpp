
#include "socket.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "logging.h"
#include <sys/ioctl.h>

namespace base {

Socket::Socket()
    : mSock(-1)
    , mSockType(eSocketType_TCP)
{
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr= INADDR_ANY;
    mHost.sin_port = 0;

    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

Socket::Socket(SOCKETTYPE eSocketType)
    : mSock(-1)
    , mSockType(eSocketType)
{
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr = INADDR_ANY;
    mHost.sin_port = 0;
    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

Socket::Socket(SOCKETTYPE eSocketType, int fd)
    : mSock(fd)
    , mSockType(eSocketType)
{
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr= INADDR_ANY;
    mHost.sin_port = 0;

    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

Socket::Socket(const Socket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mHost, &o.mHost, sizeof(mHost));
    Socket * p = (Socket*)&o;
    p->mSock = -1;
}

Socket::Socket(const Socket&& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mHost, &o.mHost, sizeof(mHost));
    Socket * p = (Socket*)&o;
    p->mSock = -1;
}

Socket& Socket::operator=(const Socket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mHost, &o.mHost, sizeof(mHost));
    Socket * p = (Socket *)&o;
    p->mSock = -1;
    return *this;
}

Socket::~Socket()
{
    reset();
}

bool Socket::create(SOCKETTYPE socketType)
{
    SOCKETTYPE ost = mSockType;
    if (socketType != eSocketType_None) {
        ost = socketType;
    }
    reset();
    mSockType = ost;
    if (mSockType == eSocketType_TCP) {
        mSock = socket(PF_INET, SOCK_STREAM, 0);
    } else if (mSockType == eSocketType_UDP) {
        mSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (mSock == -1) {
        return false;
    }
    return true;
}

int Socket::fd()
{
    return mSock;
}

std::string Socket::getRemoteIP()
{
    char ip[1024] = {0};
    inet_ntop(AF_INET, &mRemote.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(mRemote.sin_addr));
}

bool Socket::setRemoteIP(const std::string& ipaddr)
{
    // mRemote.sin_addr.s_addr = inet_addr(ipaddr);
    if (inet_aton(ipaddr.c_str(), &mRemote.sin_addr) != 0) {
        return true;
    }
    return false;
}

int Socket::getRemotePort()
{
    return static_cast<int>(ntohs(mRemote.sin_port));
}

void Socket::setRemotePort(int port)
{
    mRemote.sin_port = htons(port);
}

std::string Socket::getHostIP()
{
    char ip[1024] = {0};
    inet_ntop(AF_INET, &mHost.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(mHost.sin_addr));
}

bool Socket::setHostIP(const std::string& ipaddr)
{
    if (inet_aton(ipaddr.c_str(), &mHost.sin_addr) != 0) {
        return true;
    }
    return false;
}

bool Socket::setHostAddr(const std::string& addr)
{
    std::vector<std::string> v = split(addr);
    bool r = true;
    if (v.size() == 1) {
        r = setHostIP("0.0.0.0");
        setHostPort(atoi(v[0].c_str()));
    } else if (v.size() == 2) {
        r = setHostIP(v[0]);
        setHostPort(atoi(v[1].c_str()));
    } else if (v.size() > 2) {
        int last = v.size() - 1;
        r = setHostIP(v[last - 1]);
        setHostPort(atoi(v[last].c_str()));
    } else {
        return false;
    }
    return r;
}

bool Socket::setRemoteAddr(const std::string& addr)
{
    std::vector<std::string> v = split(addr);
    bool r = true;
    if (v.size() == 1) {
        r = setRemoteIP("0.0.0.0");
        setRemotePort(atoi(v[0].c_str()));
    } else if (v.size() == 2) {
        r = setRemoteIP(v[0]);
        setRemotePort(atoi(v[1].c_str()));
    } else if (v.size() > 2) {
        int last = v.size() - 1;
        r = setRemoteIP(v[last - 1]);
        setRemotePort(atoi(v[last].c_str()));
    } else {
        return false;
    }
    return r;

}

int Socket::getHostPort()
{
    return static_cast<int>(ntohs(mHost.sin_port));
}

void Socket::setHostPort(int port)
{
    mHost.sin_port = htons(port);
}

bool Socket::enableBroadcast()
{
    int onoff = 1;
    if (0 != setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, (char *)&onoff, sizeof(int))) {
        return false;
    }
    return true;
}

bool Socket::listen()
{
    return (::listen(mSock, 10) == 0);
}

bool Socket::bind(int port)
{
    if (port >= 0) {
        setHostPort(port);
    }
    int op = 1;
    int ret = setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
    if(ret != 0) {
        perror("setsockopt.SO_REUSEADDR");
    }
    // RUN_HERE() << getHostPort();
    ret = ::bind(mSock, (struct sockaddr *)&mHost, sizeof(mHost));
    return (ret == 0);
}

bool Socket::connect(const std::string& ipaddr, int port, int mstimeout)
{
    if (ipaddr != "") {
        setRemoteIP(ipaddr);
    }
    if (port != -1) {
        setRemotePort(port);
    }

    errno = 0;
    if(mstimeout <= 0) {
        int ret = ::connect(mSock, (struct sockaddr *)&mRemote, sizeof(mRemote));
        return (ret == 0);
    } else {
        unsigned long ul = 1;
        ioctl(mSock, FIONBIO, &ul);
        if (::connect(mSock, (struct sockaddr *)&mRemote, sizeof(mRemote)) == -1) {
            timeval tm;
            fd_set rfds;

            FD_ZERO(&rfds);
            FD_SET(mSock, &rfds);
            tm.tv_sec = mstimeout / 1000;
            tm.tv_usec = (mstimeout % 1000) * 1000;

            int ret = select(mSock + 1, NULL, &rfds, NULL, &tm);
            ul = 0;
            ioctl(mSock, FIONBIO, &ul);

            if (ret <= 0) {
                return false;
            } else {
                int error;
                int len = sizeof(int);
                getsockopt(mSock, SOL_SOCKET, SO_ERROR, (char *)&error, (socklen_t *)&len);
                if (error != 0) {
                    errno = error;
                    return false;
                }
            }
        }
        return true;
    }
}

Socket Socket::accept()
{
    socklen_t len;
    int fd;
    len = sizeof(mRemote);
    fd = ::accept(mSock, (struct sockaddr *)&mRemote, &len);

    Socket sock;
    sock.reset();
    sock.mSock = fd;
    sock.setRemoteIP(getRemoteIP());
    sock.setRemotePort(getRemotePort());
    return sock;
}

int Socket::recv(void * buf, size_t len)
{
    socklen_t sl = sizeof(mRemote);
    int ret;

    if (mSockType == eSocketType_UDP) {
        ret = ::recvfrom(mSock, (char *)buf, len, 0, (struct sockaddr *)&mRemote, &sl);
    } else {
        ret = ::read(mSock, (char *)buf, len);
    }
    if (ret <= 0) {
        onDisconnected();
    } else {
        onReadData(buf, ret);
    }
    return ret;
}

int Socket::send(const void * buf, size_t len)
{
    socklen_t sl = sizeof(mRemote);
    if (mSockType == eSocketType_UDP) {
        return ::sendto(mSock, (const char *)buf, len, 0, (struct sockaddr *)&mRemote, sl);
    }  else {
        return ::write(mSock, (const char *)buf, len);
    }
}

int Socket::recvFrom(char * ipaddr, int& port, void * buf, size_t len)
{
    int             ret;
    ret = recv(buf, len);
    strcpy(ipaddr, getRemoteIP().c_str());
    port = getRemotePort();
    return ret;
}

int Socket::sendTo(const std::string& ipaddr, int port, const void * buf, size_t len)
{
    setRemoteIP(ipaddr);
    setRemotePort(port);
    return send(buf, len);
}

void Socket::reset()
{
    if (mSock != -1) {
        close(mSock);
    }
    mSock = -1;
    mSockType = eSocketType_TCP;
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr = INADDR_ANY;
    mHost.sin_port = 0;
    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

bool Socket::isValid(void)
{
    return (mSock != -1);
}

const char * Socket::lastErrorMessage()
{
    return strerror(errno);
}

std::vector<std::string> Socket::split(const std::string& s, char delimiter)
{
    std::vector<std::string> v;
    std::string::size_type pos1, pos2;
    pos2 = s.find(delimiter);
    pos1 = 0;
    while (pos2 != std::string::npos) {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
        pos2 = s.find(delimiter, pos1);
    }
    v.push_back(s.substr(pos1));
    return v;
}

} // namespace base 

