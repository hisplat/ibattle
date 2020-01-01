
#pragma once

#include "common/worker.h"
#include "parser/parser.h"

namespace ib {

class Command;
class StdinWorker : public Worker {
public:
    StdinWorker(Worker::Device* device);
    virtual ~StdinWorker();
    DECLARE_CLASS(StdinWorker);

    // Overrided from Worker
    virtual void dump(std::ostream& o);
    virtual void onData(const void * buf, int len);
    virtual void doHeartbit();

    void write(Command* command);

protected:
    StdinWorker();
};
} // namespace ib

