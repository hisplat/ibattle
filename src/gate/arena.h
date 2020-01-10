#pragma once

#include <string>
#include <list>

namespace ib {

class GateWorker;
class Arena {
public:
    Arena() {}
    ~Arena() {}

    const std::string& name() { return mName; }
    void setName(const std::string& n) { mName = n; }

    void attachServer(GateWorker* worker);
    void detachServer(GateWorker* worker);
    void attachPlayer(GateWorker* worker);
    void detachPlayer(GateWorker* worker);

    int serverCount() { return (int)mServerWorkers.size(); }
    int playerCount() { return (int)mPlayerWorkers.size(); }

    std::list<GateWorker*>& servers() { return mServerWorkers; }
    std::list<GateWorker*>& players() { return mPlayerWorkers; }

private:
    std::string mName;
    std::list<GateWorker*> mServerWorkers;
    std::list<GateWorker*> mPlayerWorkers;
};
} // namespace ib

