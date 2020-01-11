
#include "gate.h"

#include "device/listener_device.h"
#include "device/stdin_device.h"
#include "base/logging.h"
#include "base/helper.h"
#include "parser/invoke_command.h"
#include "parser/return_command.h"
#include "parser/login_command.h"
#include "parser/server_info_command.h"
#include "arena.h"
#include <sstream>

namespace ib {

Gate::Gate()
    : mPoller(NULL)
    , mServerWorker(NULL)
    , mNodeWorker(NULL)
    , mControllerWorker(NULL)
    , mWatcherWorker(NULL)
    , mDatabaseWorker(NULL)
{
    mPoller = new Poller(this);
    mArenas.clear();
}

Gate::~Gate()
{
    destroyWorker(mServerWorker);
    destroyWorker(mNodeWorker);
    destroyWorker(mControllerWorker);
    destroyWorker(mWatcherWorker);
    destroyWorker(mDatabaseWorker);
    destroyWorker(mStdin);

    for (std::list<GateWorker*>::iterator it = mWorkers.begin(); it != mWorkers.end(); it++) {
        destroyWorker(*it);
    }

    for (std::map<std::string, Arena*>::iterator it = mArenas.begin(); it != mArenas.end(); it++) {
        delete it->second;
    }

    if (mPoller != NULL) {
        delete mPoller;
    }
}

void Gate::destroyWorker(Worker* worker)
{
    if (worker != NULL) {
        Worker::Device * device = worker->device();
        delete device;
        delete worker;
    }
}

void Gate::run(int server, int node, int controller, int watcher, int database)
{
    ListenerDevice * d1 = new ListenerDevice(server);
    mServerWorker = new Worker(d1);

    ListenerDevice * d2 = new ListenerDevice(node);
    mNodeWorker = new Worker(d2);

    ListenerDevice * d3 = new ListenerDevice(controller);
    mControllerWorker = new Worker(d3);

    ListenerDevice * d4 = new ListenerDevice(watcher);
    mWatcherWorker = new Worker(d4);

    ListenerDevice * d5 = new ListenerDevice(database);
    mDatabaseWorker = new Worker(d5);

    StdinDevice * d6 = new StdinDevice();
    mStdin = new StdinWorker(d6);

    mPoller->addWorker(mServerWorker);
    mPoller->addWorker(mNodeWorker);
    mPoller->addWorker(mControllerWorker);
    mPoller->addWorker(mWatcherWorker);
    mPoller->addWorker(mDatabaseWorker);
    mPoller->addWorker(mStdin);

    mPoller->run();
}

void Gate::stop()
{
    mPoller->stop();
}


void Gate::dump(std::ostream& o)
{
}

void Gate::onCommand(Worker* worker, Command* cmd)
{
    if (!cmd->isClass("HeartbitCommand")) {
        RUN_HERE() << "received " << cmd << " from " << worker;
    }

    Command::CommandType type = cmd->type();
    Command::CommandAction action = cmd->action();

    if (worker == mStdin) {
        onControlCommand(worker, cmd);
    } else {
        if (!worker->isClass("GateWorker")) {
            FATAL() << "Unkown worker type.";
            return;
        }

        GateWorker * gw = (GateWorker*)worker;
        switch(gw->type()) {
        case GateWorker::eType_Server:
            onServerCommand(gw, cmd);
            break;
        case GateWorker::eType_Controller:
            onControlCommand(worker, cmd);
            break;
        case GateWorker::eType_Node:
            onNodeCommand(gw, cmd); 
            break;
        case GateWorker::eType_Watcher:
        case GateWorker::eType_Database:
            FATAL() << "Not implemented yet.";
            break;
        default:
            FATAL() << "should never run here.";
            break;
        }
    }
}

void Gate::onNewConnection(Worker* worker, void* arg)
{
    base::Socket * socket = (base::Socket*)arg;
    NetDevice * device = new NetDevice(socket);
    GateWorker * ww = new GateWorker(device);

    if (worker == mServerWorker) {
        ww->setType(GateWorker::eType_Server);
    } else if (worker == mNodeWorker) {
        ww->setType(GateWorker::eType_Node);
    } else if (worker == mControllerWorker) {
        ww->setType(GateWorker::eType_Controller);
    } else if (worker == mDatabaseWorker) {
        ww->setType(GateWorker::eType_Database);
    } else {
        FATAL() << "should never run here.";
    }

    mPoller->addWorker(ww);
    mWorkers.push_back(ww);
    IMPORTANT() << "client connected: " << ww;
}

void Gate::onDisconnect(Worker* worker)
{
    mPoller->removeWorker(worker);

    if (worker->isClass("GateWorker")) {
        GateWorker * gw = (GateWorker*)worker;
        if (gw->type() == GateWorker::eType_Server) {
            onGameWorkerOffline(gw);
        } else if (gw->type() == GateWorker::eType_Node) {
            onPlayerOffline(gw);
        }

        for (std::list<GateWorker*>::iterator it = mWorkers.begin(); it != mWorkers.end(); /* */) {
            if (*it == worker) {
                it = mWorkers.erase(it);
            } else {
                it++;
            }
        }
    }
    destroyWorker(worker);
}

void Gate::onTimeout(Worker* worker)
{
}

void Gate::onHeartbit(Worker* worker)
{
}

void Gate::onFileChanged(Worker* worker, const std::string& path, int mask)
{
}

void Gate::onControlCommand(Worker* worker, Command* command)
{
    LCHECK(strcmp(command->getClassName(), "InvokeCommand") == 0);

    InvokeCommand * control = (InvokeCommand*)command;
    const std::string& s = control->data();
    std::vector<std::string> args = base::helper::split(s);
    if (args.size() < 1) {
        return;
    }

    std::string c = base::helper::trim(args[0]);

    if (c == "") {
        return;
    }

    ReturnCommand ret;
    if (c == "quit") {
        stop();
        return;
    } else if (c == "list") {
        std::stringstream ss;
        ss << "STATUS OF ARENAS:" << std::endl;
        int index = 1;
        for (std::map<std::string, Arena*>::iterator it = mArenas.begin(); it != mArenas.end(); it++, index++) {
            Arena* sp = it->second;
            ss << "Arena #" << index << ": " << sp->name() << "(" << sp->playersPerGame() << ")" << std::endl;
            ss << "     Servers: " << sp->serverCount() << std::endl;
            ss << "     Players: " << sp->playerCount() << std::endl;

            auto players = sp->players();
            int pi = 1;
            for (GateWorker * gw : players) {
                ss << "        " << pi << ": " << gw->uuid() << std::endl;
                pi++;
            }
        }
        ret.setData(ss.str());
    } else {
        std::string tip("unknown command: ");
        tip += s;
        ret.setData(tip);
    }

    if (strcmp(worker->getClassName(), "StdinWorker") == 0) {
        mStdin->write(&ret);
    } else {
        ((GateWorker*)worker)->write(&ret);
    }
}

void Gate::onServerCommand(GateWorker* worker, Command* command)
{
    if (command->isClass("LoginCommand")) {
        LoginCommand * lc = (LoginCommand*)command;
        std::string uuid = lc->uuid();
        std::string name = lc->name();
        worker->setUuid(uuid);
        worker->setName(name);
        onGameWorkerOnline(worker);
    } else if (command->isClass("ServerInfoCommand")) {
        ServerInfoCommand * sic = (ServerInfoCommand*)command;
        switch (sic->op()) {
        case ServerInfoCommand::eOp_SetPlayerCount:
            {
                std::string name = worker->name();
                Arena * arena = findArena(name);
                if (arena == NULL) {
                    IMPORTANT() << "A server trying to set player count before login: " << worker;
                    return;
                }
                ServerActionSetPlayerCount sasc;
                sasc.loadFrom(sic->data());
                arena->setPlayersPerGame(sasc.playercount);
            }
            break;
        case ServerInfoCommand::eOp_Score:
            break;
        default:
            FATAL() << "should not run here.";
        }
    }
}

void Gate::onNodeCommand(GateWorker* worker, Command* command)
{
    if (command->isClass("LoginCommand")) {
        LoginCommand * lc = (LoginCommand*)command;
        std::string uuid = lc->uuid();
        std::string name = lc->name();
        worker->setUuid(uuid);
        worker->setName(name);
        onPlayerOnline(worker);
    }
}

Arena * Gate::findArena(const std::string& name)
{
    Arena* gp = nullptr;
    std::map<std::string, Arena*>::iterator it = mArenas.find(name);
    if (it == mArenas.end()) {
        gp = new Arena();
        gp->setName(name);
        mArenas[name] = gp;
    } else {
        gp = it->second;
    }

    LCHECK(gp != NULL);
    return gp;
}

void Gate::onGameWorkerOnline(GateWorker* worker)
{
    std::string name = worker->name();
    Arena* arena = findArena(name);;
    arena->attachServer(worker);
}

void Gate::onGameWorkerOffline(GateWorker* worker)
{
    std::string name = worker->name();
    if (name == "") {
        ALERT() << "game server disconnected without ever attached: " << worker;
        return;
    }
    Arena* arena = findArena(name);;
    arena->detachServer(worker);
}

void Gate::onPlayerOnline(GateWorker* worker)
{
    std::string name = worker->name();
    Arena* arena = findArena(name);;
    arena->attachPlayer(worker);
}

void Gate::onPlayerOffline(GateWorker* worker)
{
    std::string name = worker->name();
    if (name == "") {
        ALERT() << "player disconnected without ever attached: " << worker;
        return;
    }
    Arena* arena = findArena(name);;
    arena->detachPlayer(worker);
}

} // namespace ib

