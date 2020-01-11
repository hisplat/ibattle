#pragma once

#include <string>
#include <map>
#include <vector>
#include <pthread.h>

#include "base/buffer.h"
#include "base/condition.h"
#include "base/pipe_waiter.h"
#include "base/condition.h"
#include "base/lock.h"


namespace ib {
class Client;
class Condition;
class CallbackHelper;
class Command;
class Battle {
public:
    class Handler {
    public:
        Handler() {}
        virtual ~Handler() {}
        virtual void onCommand(const std::string& command, base::Buffer& ret) = 0;
        virtual void onConnected() {}
        virtual void onDisconnected() {}
    };

    typedef enum {
        eRet_Success = 0,
        eRet_NotInited = 1,
        eRet_TimeOut,
        eRet_Disconnected,
    } RetCode;

    Battle();
    ~Battle();
    
    bool start(const std::string& ip, int port, const std::string& name, Handler* handler);
    void stop();

    RetCode call(const std::string& destination, const std::string& command);
    RetCode call(const std::string& destination, const std::string& command, base::Buffer& ret, int timeout = 3000);

    RetCode call(Command * command);
    RetCode call(Command * command, base::Buffer& ret, int timeout = 3000);

    RetCode call(Command& command) { return call(&command); }
    RetCode call(Command& command, base::Buffer& ret, int timeout = 3000) { return call(&command, ret, timeout); }

public:
    void onCall(const std::string& command, base::Buffer& ret);
    void onReturn(int cid, base::Buffer& ret, int errorcode);
    void onConnected();
    void onDisconnected();
    bool onTimer(void* arg, int count);

    Handler* handler() { return mHandler; }

private:
    static void* __routine(void* arg);
    std::string strRetCode(RetCode code);
    RetCode error2ret(int errcode);

    bool mInited;
    Client * mClient;
    Handler* mHandler;
    std::string mIp;
    int mPort;
    std::string mUuid;
    std::string mName;

    class Waiter {
    public:
        Waiter();
        Waiter(base::Buffer* ret);
        ~Waiter();
        void wait();
        void wait(int timeout_ms);
        void notify();
        void setRet(base::Buffer& buf);
        void setErrorCode(Battle::RetCode err) { mErrorCode = err; }
        Battle::RetCode errorCode() { return mErrorCode; }
        base::Condition& condition() { return mCondition; }
    private:
        base::Buffer* mRetBuffer;
        base::Condition mCondition;
        Battle::RetCode mErrorCode;
    };
    std::map<int, Waiter*> mSyncCalls;
    pthread_key_t mThreadKey;
    base::PipeWaiter mInitWaiter;
    base::Lock mSyncCallsLock;
    pthread_t mThread;
    CallbackHelper* mHelper;

    void wakeUpAllWaiter();
    void runClient();
};

} // namespace hirpc


