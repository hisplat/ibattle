
#pragma once
#include "worker.h"

namespace ib {

class RtmgrDevice : public Worker::Device {
public:
    RtmgrDevice();
    virtual ~RtmgrDevice();
    DECLARE_CLASS(RtmgrDevice);

    virtual void onDataArrival(int fd);
    virtual void write(const void * buf, int len);
    virtual int getFd();
    virtual void disconnect();


private:
    int mSocket;
};

#define FORWARD_NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
                   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
                   (int)(nlh)->nlmsg_len <= (len))

} // namespace ib

