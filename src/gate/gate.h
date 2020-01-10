
#pragma once


#include "base/helper.h"
#include "common/poller.h"
#include "gate_worker.h"
#include "parser/stdin_worker.h"
#include <string>
#include <map>

namespace ib {

class Arena;
class LoginCommand;
class Gate : public Poller::Executor {
public:
    Gate();
    virtual ~Gate();
    DECLARE_CLASS(Gate);

    void run(int server = 20100, int node = 20101, int controller = 20102, int watcher = 20103, int database = 20104);
    void stop();
    virtual void dump(std::ostream& o);

    // implemented from Poller::Executor.
    virtual void onCommand(Worker* worker, Command* cmd);
    virtual void onNewConnection(Worker * worker, void* arg);
    virtual void onDisconnect(Worker* worker);
    virtual void onTimeout(Worker* worker);
    virtual void onHeartbit(Worker* worker);
    virtual void onFileChanged(Worker* worker, const std::string& path, int mask);

private:
    // instrument functions.
    void destroyWorker(Worker* worker);

private:
    Poller * mPoller;

    Worker * mServerWorker;
    Worker * mNodeWorker;
    Worker * mControllerWorker;
    Worker * mWatcherWorker;
    Worker * mDatabaseWorker;
    StdinWorker* mStdin;

    std::list<GateWorker*> mWorkers;

    std::map<std::string, Arena*> mArenas;


    void onControlCommand(Worker* worker, Command* command);
    void onServerCommand(GateWorker* worker, Command* command);
    void onNodeCommand(GateWorker* worker, Command* command);

    Arena * findArena(const std::string& name);

    void onGameWorkerOnline(GateWorker* worker);
    void onGameWorkerOffline(GateWorker* worker);
    void onPlayerOnline(GateWorker* worker);
    void onPlayerOffline(GateWorker* worker);
};

} // namespace ib

