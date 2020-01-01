
#include "parser_worker.h"
#include "base/dump.h"

#include "parser/command.h"
#include "parser/command_parser.h"
#include "common/poller.h"

namespace ib {

ParserWorker::ParserWorker(Worker::Device* device)
    : Worker(device)
{
}

ParserWorker::ParserWorker()
    : Worker()
{
}

ParserWorker::~ParserWorker()
{
}

void ParserWorker::dump(std::ostream& o)
{
    Worker::dump(o);
}

void ParserWorker::onData(const void * buf, int len)
{
    base::dump(buf, len);

    Worker::onData(buf, len); // update alive time.

    for (int ret = parse(buf, len); ret >= 0; ret = parse(NULL, 0)) {
        // packet data will be send to 'onPacketParsed'.
    }
}

void ParserWorker::doHeartbit()
{
}

void ParserWorker::onPacketParsed(const void * data, int len, int data_offset)
{
    LCHECK(len >= data_offset);
    const char * p = (const char *)data;
    p += data_offset;
    len -= data_offset;

    Command * command = CommandParser::makeCommand(p, len);
    poller()->onCommand(this, command);
    delete command;
}

void ParserWorker::write(Command* command)
{
    base::Buffer buf = createPacket(command);

    Worker::write(buf.buffer(), buf.length());
}

} // namespace ib

