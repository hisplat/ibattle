#include <stdio.h>
#include <algorithm>
#include <dirent.h>
#include "client.h"
#include "base/buffer.h"
#include "base/helper.h"
#include "base/logging.h"
#include "device/net_device.h"
#include "base/socket.h"

namespace ib {

Client::Client()
    : mPoller(NULL)
    , mWorker(NULL)
    , mCallback(NULL)
    , mCurrentCid(1000)
{
    mPoller = new Poller(this);
}

Client::Client(Client::Callback* callback)
    : mPoller(NULL)
    , mWorker(NULL)
    , mCallback(callback)
    , mCurrentCid(1000)
{
    mPoller = new Poller(this);
}

Client::~Client()
{
    if (mPoller != NULL) {
        delete mPoller;
    }
    destroyWorker(mWorker);
}

void Client::destroyWorker(Worker* worker)
{
    if (worker != NULL) {
        Worker::Device * device = worker->device();
        delete device;
        delete worker;
    }
}

void Client::run(const std::string& ip, int port, const std::string& token)
{
    base::Socket s;
    bool ret = s.create();
    LCHECK(ret == true) << "create socket failed.";

    ret = s.connect(ip, port);
    LCHECK(ret == true) << "connect to server: " << ip << ":" << port << " failed.";

    NetDevice * device = new NetDevice(s);
    mWorker = new ClientWorker(device);

    mPoller->addWorker(mWorker);

    if (mCallback != NULL) {
        mCallback->onConnected();
    }

    mPoller->run();
}

void Client::stop()
{
    mPoller->stop();
}

void Client::onCommand(Worker* worker, Command* cmd)
{
    RUN_HERE() << cmd;

    Command::CommandType type = cmd->type();
    Command::CommandAction action = cmd->action();

    switch (action) {
    case Command::eAction_Invoke:
    case Command::eAction_Ret:
    case Command::eAction_Ack:
    case Command::eAction_Heartbit:
    case Command::eAction_Game:
    case Command::eAction_Play:
    case Command::eAction_Control:
    case Command::eAction_Live:
    case Command::eAction_Database:
    case Command::eAction_Login:
        break;
    default:
        break;
    }

}

void Client::onDisconnect(Worker* worker)
{
    NFLOG() << "Server disconnected: " << worker;
    mPoller->removeWorker(worker);
    destroyWorker(worker);
}

void Client::onTimeout(Worker* worker)
{
    NFLOG() << "server timeout: " << worker;
    onDisconnect(worker);
}

int Client::invoke(Command* command)
{
    if (mCurrentCid > 32760) {
        ALERT() << "The cid has reached the limit(32760), reset it.";
        mCurrentCid = 0;
    }
    int cid = mCurrentCid++;
    cid = (cid & 0xffff);

    command->setCid(cid);
    mWorker->write(command);

    NFLOG() << "send command " << command;
    return cid;
}


void Client::onDatabaseCommand(Worker* worker, Command* command)
{
}

void Client::onLoginCommand(Worker* worker, Command* command)
{
}

Client::ClientWorker::ClientWorker(Device* device)
    : ParserWorker(device)
{
}

Client::ClientWorker::ClientWorker()
    : ParserWorker()
{
}

Client::ClientWorker::~ClientWorker()
{
}


} // namespace ib 

