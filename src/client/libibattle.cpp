
#include <pthread.h>

#include "libibattle.h"

#include "base/buffer.h"
#include "base/condition.h"
#include "base/logging.h"
#include "base/helper.h"

#include "client.h"
#include "parser/invoke_command.h"
#include "parser/login_command.h"

namespace ib {

class CallbackHelper : public Client::Callback, public Timer {
public:
    CallbackHelper(Battle* rpc) : mBattle(rpc) {}
    virtual ~CallbackHelper() {}
    virtual void onCall(const std::string& command, base::Buffer& ret) {
        mBattle->onCall(command, ret);
    }
    virtual void onReturn(int cid, base::Buffer& ret, int errorcode) {
        mBattle->onReturn(cid, ret, errorcode);
    }
    virtual void onConnected() {
        mBattle->onConnected();
    }
    virtual void onDisconnected() {
        mBattle->onDisconnected();
    }
    virtual bool onTimer(void* arg, int count) {
        return mBattle->onTimer(arg, count);
    }
private:
    Battle* mBattle;
};


Battle::Battle()
    : mInited(false)
    , mClient(NULL)
    , mHandler(NULL)
    , mHelper(NULL)
{
    mHelper = new CallbackHelper(this);
    mClient = new Client(mHelper);

    pthread_key_create(&mThreadKey, NULL);
    pthread_setspecific(mThreadKey, (void*)0);
}

Battle::~Battle()
{
    if (mClient != NULL) {
        delete mClient;
    }
    if (mHelper != NULL) {
        delete mHelper;
    }
}

void * Battle::__routine(void* arg)
{
    logging::setThreadName("iBattleClient");
    VERBOSE() << "ibattle I/O thread created.";
    Battle* b = (Battle*)arg;
    b->runClient();
    return NULL;
}

void Battle::runClient()
{
    mClient->run(mIp, mPort, mToken);
}

bool Battle::start(const std::string& ip, int port, const std::string& token, Handler* handler)
{
    if (mInited) {
        return false;
    }

    mHandler = handler;
    mIp = ip;
    mPort = port;
    mToken = token;

    pthread_create(&mThread, NULL, __routine, this);
    mInited = true;
    mInitWaiter.wait();
    VERBOSE() << "ibattle inited. trying login.";

    LoginCommand login;
    login.setToken(token);
    mClient->invoke(&login);

    return true;
}

void Battle::stop()
{
    mClient->stop();
    pthread_join(mThread, NULL);
}

Battle::RetCode Battle::call(const std::string& destination, const std::string& command, base::Buffer& ret, int timeout)
{
    long nested = (long)pthread_getspecific(mThreadKey);
    if (nested == 1) {
        FATAL() << "nested call has not been implemented yet.";
        return eRet_NotInited;
    }

    // use condition directly: received ReturnCommand before this thread reaches waiter.wait.
    Waiter waiter(&ret);
    base::Condition& condition = waiter.condition();
    condition.lock();
    waiter.setErrorCode(eRet_TimeOut); // errorcode must be set before invoke(return).

    mSyncCallsLock.acquire(); // sometimes net-thread runs faster than current thread, so invoke() and onReturn() should be synchronized.


    InvokeCommand invokeCommand;
    invokeCommand.setData(command);
    
    int cid = mClient->invoke(&invokeCommand);

    mSyncCalls[cid] = &waiter;
    mSyncCallsLock.release();

    IMPORTANT() << "waitting 'return' or 'ack' command of sync call before timeout: " << timeout;
    long begin = base::helper::mtick();
    if (timeout > 0) {
        void * arg = reinterpret_cast<void*>(cid);
        int timerid = mClient->poller()->addTimer(mHelper, timeout, arg);
        condition.wait();
        condition.unlock();
        mClient->poller()->removeTimer(timerid);
    } else {
        condition.wait();
        condition.unlock();
    }
    long now = base::helper::mtick();
    IMPORTANT() << "the last sync call of [" << &invokeCommand << "] costs " << (now - begin) << "/" << timeout << " ms. return code is " << waiter.errorCode() << " which means '" << strRetCode(waiter.errorCode()) << "'";

    mSyncCallsLock.acquire();
    std::map<int, Waiter*>::iterator it = mSyncCalls.find(cid);
    if (it != mSyncCalls.end()) {
        mSyncCalls.erase(it);
    }
    mSyncCallsLock.release();

    return waiter.errorCode();
}

Battle::RetCode Battle::call(const std::string& destination, const std::string& command)
{
    InvokeCommand invokeCommand;
    invokeCommand.setData(command);
    mClient->invoke(&invokeCommand);
    return eRet_Success;
}



void Battle::onCall(const std::string& command, base::Buffer& ret)
{
    // TODO: need a new thread.
    if (mHandler != NULL) {
        pthread_setspecific(mThreadKey, (void*)1);
        mHandler->onCommand(command, ret);
        pthread_setspecific(mThreadKey, (void*)0);
    }
}

void Battle::onReturn(int cid, base::Buffer& ret, int errorcode)
{
    // RUN_HERE();
    base::AutoLock l(mSyncCallsLock);
    // RUN_HERE();
    std::map<int, Waiter*>::iterator it = mSyncCalls.find(cid);
    if (it == mSyncCalls.end()) {
        // RUN_HERE();
        ALERT() << "Recieved a return command but there's no relatived waiter. maybe server timeout?";
        return;
    }
    Waiter * waiter = it->second;
    waiter->setRet(ret);
    waiter->setErrorCode(error2ret(errorcode));
    waiter->notify();
}

void Battle::onConnected()
{
    mInitWaiter.notify();
    if (mHandler != NULL) {
        mHandler->onConnected();
    }
}

void Battle::onDisconnected()
{
    NFLOG() << "server disconnected. wake up all sync calls.";
    wakeUpAllWaiter();
    if (mHandler != NULL) {
        mHandler->onDisconnected();
    }
}

bool Battle::onTimer(void* arg, int count)
{
    int cid = (reinterpret_cast<long>(arg) & 0xffffffff);

    RUN_HERE() << "cid " << cid << " timeout.";
    mSyncCallsLock.acquire();
    std::map<int, Waiter*>::iterator it = mSyncCalls.find(cid);
    if (it != mSyncCalls.end()) {
        Waiter* waiter = it->second;
        waiter->setErrorCode(eRet_TimeOut);
        waiter->notify();
        mSyncCalls.erase(it);
    }
    mSyncCallsLock.release();
    return false;
}

Battle::RetCode Battle::error2ret(int errcode)
{
    return eRet_Success;
}

Battle::Waiter::Waiter()
    : mRetBuffer(NULL)
    , mErrorCode(Battle::eRet_Success)
{
}

Battle::Waiter::Waiter(base::Buffer* ret)
    : mRetBuffer(ret)
    , mErrorCode(Battle::eRet_TimeOut)
{
}

Battle::Waiter::~Waiter()
{
}

void Battle::Waiter::wait()
{
    DEBUG() << "waiting sync call...";
    mCondition.lock();
    mCondition.wait();
    mCondition.unlock();
}
void Battle::Waiter::wait(int timeout_ms)
{
    DEBUG() << "waiting sync call...";
    struct timespec abstime;
    struct timeval now;
    gettimeofday(&now, NULL);
    int nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
    abstime.tv_nsec = nsec % 1000000000;
    abstime.tv_sec = now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
    mCondition.lock();
    mCondition.wait(&abstime);
    mCondition.unlock();
}

void Battle::Waiter::notify()
{
    DEBUG() << "notify sync call...";
    mCondition.lock();
    mCondition.signal();
    mCondition.unlock();
}

void Battle::Waiter::setRet(base::Buffer& buf)
{
    mRetBuffer->move(buf);
}

void Battle::wakeUpAllWaiter()
{
    base::AutoLock l(mSyncCallsLock);
    for (std::map<int, Waiter*>::iterator it = mSyncCalls.begin(); it != mSyncCalls.end(); it++) {
        Waiter * waiter = it->second;
        waiter->setErrorCode(Battle::eRet_Disconnected);
        waiter->notify();
    }
    NFLOG() << "Waken up " << mSyncCalls.size() << " calls.";
}

std::string Battle::strRetCode(Battle::RetCode code)
{
    switch (code) {
    case Battle::eRet_Success:
        return "Success.";
    case Battle::eRet_NotInited:
        return "Not inited. you should call 'start' before calling any command.";
    case Battle::eRet_TimeOut:
        return "Timeout.";
    default:
        return "unknown";
    }
}

} // namespace ib


