
#include "gate_worker.h"
#include "base/dump.h"

#include "parser/command.h"
#include "parser/command_parser.h"
#include "common/poller.h"

namespace ib {

GateWorker::GateWorker(Worker::Device* device)
    : ParserWorker(device)
{
}

GateWorker::GateWorker()
    : ParserWorker()
{
}

GateWorker::~GateWorker()
{
}

void GateWorker::dump(std::ostream& o)
{
    o << "Type: " << mType << ", Uuid: " << mUuid << ", Name: " << mName;
}

} // namespace ib

