#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <vector>

namespace base {

class UnixSocket {
public:
    typedef enum {
        eSocketType_None = 0,
        eSocketType_Stream,
        eSocketType_Dgram,
    } SOCKETTYPE;

    UnixSocket();
    UnixSocket(SOCKETTYPE eSocketType);
    UnixSocket(SOCKETTYPE eSocketType, int fd);
    UnixSocket(const UnixSocket& o);
    UnixSocket(const UnixSocket&& o);
    virtual ~UnixSocket();
    UnixSocket& operator=(const UnixSocket& o);

    bool isValid();
    void reset();

    // true: success.
    // false: fail.
    bool create(SOCKETTYPE socketType = eSocketType_None);
    bool setRemoteAddr(const std::string& addr);
    bool setLocalAddr(const std::string& ipaddr);
    bool listen();
    bool bind(const std::string& addr = "");
    bool connect(const std::string& addr = "");
    UnixSocket accept();

    int fd();
    std::string getRemoteAddr();
    std::string getLocalAddr();

    int recv(void * buf, size_t len);
    int send(const void * buf, size_t len);

    const char * lastErrorMessage();

public:
    virtual void onReadData(const void * buf, int len) {}
    virtual void onDisconnected(void) {}


private:
    int mSock;
    SOCKETTYPE mSockType;
    sockaddr_un mRemote;
    sockaddr_un mLocal;
};

} // namespace base

