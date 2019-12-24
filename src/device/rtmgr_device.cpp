#include "rtmgr_device.h"
#include "base/logging.h"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>

namespace ib {

RtmgrDevice::RtmgrDevice()
    : mSocket(-1)
{
    struct sockaddr_nl sa; 
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    mSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    LCHECK(mSocket >= 0);
    if (mSocket < 0) {
        PERROR() << "create socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE) failed.";
        mSocket = -1;
        return;
    }
    int ret = bind(mSocket, (struct sockaddr *)&sa, sizeof(sa));
    LCHECK(ret == 0);
    if (ret < 0) {
        PERROR() << "bind netlink socket failed.";
        return;
    }
}

RtmgrDevice::~RtmgrDevice()
{
    if (mSocket >= 0) {
        close(mSocket);
    }
}

void RtmgrDevice::onDataArrival(int fd)
{
    if (mSocket < 0) {
        return;
    }

    LCHECK(fd == mSocket);

    char buffer[4096];

    struct iovec iov;
    iov.iov_base = buffer;
    iov.iov_len = sizeof(buffer);

    struct sockaddr_nl sa; 
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    struct msghdr msg;
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    int ret = recvmsg(mSocket, &msg, 0);
    for (struct nlmsghdr * nh = (struct nlmsghdr *)buffer; FORWARD_NLMSG_OK(nh, ret); nh = NLMSG_NEXT(nh, ret)) {
        if (nh->nlmsg_type == NLMSG_DONE || nh->nlmsg_type == NLMSG_ERROR) {
            break;
        }

        void * info = NLMSG_DATA(nh);
        struct ifinfomsg * ifi = (struct ifinfomsg *)info;
        struct rtattr* rta = (struct rtattr *)((char *)ifi + NLMSG_ALIGN(sizeof(struct ifinfomsg)));
        int len = NLMSG_PAYLOAD(nh, sizeof(struct ifinfomsg));
        while (RTA_OK(rta, len)) {
            if (rta->rta_type == IFLA_IFNAME) {
                std::string ifname = (char *)RTA_DATA(rta);
                //NFLOG() << ifname << ": " << ifi->ifi_type << ", " << ifi->ifi_index << ", " << std::hex << ifi->ifi_flags << std::dec << ", " << ifi->ifi_change;
            }
            rta = RTA_NEXT(rta, len);
        }
    }

    onNetworkChanged();
}

void RtmgrDevice::write(const void * buf, int len)
{
}

int RtmgrDevice::getFd()
{
    return mSocket;
}

void RtmgrDevice::disconnect()
{
}

} // namespace ib

