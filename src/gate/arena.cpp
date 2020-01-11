
#include "arena.h"

namespace ib {

Arena::Arena()
    : mPlayersPerGame(0)
{
}

Arena::~Arena()
{
}

void Arena::attachServer(GateWorker* worker)
{
    mServerWorkers.push_back(worker);
}

void Arena::detachServer(GateWorker* worker)
{
    for (std::list<GateWorker*>::iterator it = mServerWorkers.begin(); it != mServerWorkers.end(); /* */) {
        if (*it == worker) {
            it = mServerWorkers.erase(it);
        } else {
            it++;
        }
    }
}

void Arena::attachPlayer(GateWorker* worker)
{
    mPlayerWorkers.push_back(worker);
}

void Arena::detachPlayer(GateWorker* worker)
{
    for (std::list<GateWorker*>::iterator it = mPlayerWorkers.begin(); it != mPlayerWorkers.end(); /* */) {
        if (*it == worker) {
            it = mPlayerWorkers.erase(it);
        } else {
            it++;
        }
    }
}



} // namespace ib

