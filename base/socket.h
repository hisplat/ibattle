#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string>
#include <vector>

namespace base {

class Socket {
public:
    typedef enum {
        eSocketType_None = 0,
        eSocketType_TCP,
        eSocketType_UDP,
    }SOCKETTYPE;

    Socket();
    Socket(SOCKETTYPE eSocketType);
    Socket(SOCKETTYPE eSocketType, int fd);
    Socket(const Socket& o);
    Socket(const Socket&& o);
    virtual ~Socket();
    Socket& operator=(const Socket& o);

    bool isValid();
    void reset();

    // true: success.
    // false: fail.
    bool create(SOCKETTYPE socketType = eSocketType_None);
    bool setRemoteIP(const std::string& ipaddr);
    bool setHostIP(const std::string& ipaddr);
    bool setHostAddr(const std::string& addr);
    bool setRemoteAddr(const std::string& addr);
    bool enableBroadcast();
    bool listen();
    bool bind(int port = -1);
    bool connect(const std::string& ipaddr = "", int port = -1, int mstimeout = 0);
    Socket accept();

    int fd();
    std::string getRemoteIP();
    std::string getHostIP();
    int getRemotePort();
    int getHostPort();
    void setRemotePort(int port);
    void setHostPort(int port);

    int recv(void * buf, size_t len);
    int send(const void * buf, size_t len);
    int recvFrom(char * ipaddr, int& port, void * buf, size_t len);
    int sendTo(const std::string& ipaddr, int port, const void * buf, size_t len);

    const char * lastErrorMessage();

public:
    virtual void onReadData(const void * buf, int len) {}
    virtual void onDisconnected(void) {}


private:
    int mSock;
    SOCKETTYPE mSockType;
    sockaddr_in mRemote;
    sockaddr_in mHost;
    std::vector<std::string> split(const std::string& s, char delimiter = ':');
};

} // namespace base

