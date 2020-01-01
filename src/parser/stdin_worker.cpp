
#include "stdin_worker.h"
#include "base/dump.h"

#include "parser/command.h"
#include "parser/command_parser.h"
#include "parser/control_command.h"
#include "common/poller.h"

namespace ib {

StdinWorker::StdinWorker(Worker::Device* device)
    : Worker(device)
{
}

StdinWorker::StdinWorker()
    : Worker()
{
}

StdinWorker::~StdinWorker()
{
}

void StdinWorker::dump(std::ostream& o)
{
    Worker::dump(o);
}

void StdinWorker::onData(const void * buf, int len)
{
    Worker::onData(buf, len); // update alive time.

    std::string cmd((const char *)buf);

    ControlCommand control;
    control.setCommand(cmd);
    poller()->onCommand(this, &control);
}

void StdinWorker::doHeartbit()
{
}


void StdinWorker::write(Command* command)
{
    if (strcmp(command->getClassName(), "ControlCommand") == 0) {
        const std::string& s = ((ControlCommand*)command)->command();
        Worker::write(s.c_str(), s.length() + 1);
    } else {
        IMPORTANT() << command;
    }
}

} // namespace ib

