
#include "client/libibattle.h"
#include "base/logging.h"

class ServerHandler : public ib::Battle::Handler {
public:
    ServerHandler() : ib::Battle::Handler() {}
    virtual ~ServerHandler() {}

    virtual void onCommand(const std::string& command, base::Buffer& ret) {
        RUN_HERE() << command;
    }
    virtual void onConnected() {
        RUN_HERE();
    }
    virtual void onDisconnected() {
        RUN_HERE();
    }
};

int main()
{
    ib::Battle battle;
    ServerHandler handler;

    battle.start("127.0.0.1", 20100, "simple-card", &handler);

    return 0;
}

