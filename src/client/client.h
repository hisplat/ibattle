
#pragma once

#include "parser/command.h"
#include "parser/parser_worker.h"
#include "common/poller.h"
#include "common/timer.h"
#include "common/worker.h"
#include "base/lock.h"
#include "base/buffer.h"
#include <map>
#include <list>
#include <vector>
#include <string>


namespace ib {

class ScanTimer;
class Client : public Poller::Executor {
public:
    class Callback {
    public:
        Callback() {}
        virtual ~Callback() {}
        virtual void onCall(const std::string& command, base::Buffer& ret) = 0;
        virtual void onReturn(int cid, base::Buffer& ret, int errorcode) = 0;
        virtual void onConnected() {}
        virtual void onDisconnected() {}
    };

    Client(Callback* callback);
    virtual ~Client();

    // implemented from Poller::Executor.
    virtual void onCommand(Worker* worker, Command* cmd);
    virtual void onDisconnect(Worker* worker);
    virtual void onTimeout(Worker* worker);


    void run(const std::string& ip, int port, const std::string& token);
    void stop();

    int invoke(Command* command);
    Poller* poller() { return mPoller; }

private:
    class ClientWorker : public ParserWorker {
    public:
        ClientWorker(Device* device);
        virtual ~ClientWorker();
        DECLARE_CLASS(ClientWorker);
    protected:
        ClientWorker();
    };

    Client();
    void destroyWorker(Worker* worker);

    Poller * mPoller;
    ClientWorker * mWorker;
    Callback * mCallback;
    int mCurrentCid;


private:
    // on command.
    void onDatabaseCommand(Worker* worker, Command* command);
    void onLoginCommand(Worker* worker, Command* command);
};

} // namespace ib


