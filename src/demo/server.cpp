
#include "client/libibattle.h"
#include "base/logging.h"
#include <unistd.h>

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

int main(int argc, char * argv[])
{
    logging::setLogLevel(logging::kLogLevel_Verbose);

    ib::Battle battle;
    ServerHandler handler;
    

    std::string name = "simple-card";
    if (argc > 1) {
        name = argv[1];
    }

    battle.start("127.0.0.1", 20100, name, &handler);

    while (true) {
        char temp[1024];
        fgets(temp, sizeof(temp) - 1, stdin);
        temp[strlen(temp) - 1] = '\0';
        if (strcmp(temp, "quit") == 0) {
            break;
        }
    }
    return 0;
}

