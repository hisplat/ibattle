
#pragma once

#include "common/worker.h"
#include "parser/parser.h"

namespace ib {

class ParserWorker : public Worker, public Parser {
public:
    ParserWorker(Worker::Device* device);
    virtual ~ParserWorker();
    DECLARE_CLASS(ParserWorker);

    // Overrided from Worker
    virtual void dump(std::ostream& o);
    virtual void onData(const void * buf, int len);
    virtual void doHeartbit();

    // Overrided from Parser
    virtual void onPacketParsed(const void * data, int len, int data_offset);

protected:
    ParserWorker();
};
} // namespace ib

