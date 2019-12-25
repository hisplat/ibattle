
#include "command.h"

namespace ib {


Command::Command()
    : mCid(-1)
    , mSid(-1)
    , mType(eType_Async)
    , mAction(eAction_Invoke)
{
}

Command::Command(CommandType type, CommandAction action)
    : mCid(-1)
    , mSid(-1)
    , mType(type)
    , mAction(action)
{
}

Command::~Command()
{
}


} // namespace ib

std::ostream& operator<<(std::ostream& o, ib::Command* command)
{
    if (command == NULL) {
        o << "Command: NULL";
        return o;
    }
    o << std::dec;
    o << command->getClassName() << ": {"
        << "Pointer: " << (void*)command << ", "
        << "Class: " << command->getClassName() << ", "
        << "Type: " << std::hex << command->type() << ", " << std::dec
        << "Action: " << std::hex << command->action() << ", " << std::dec
        << "Cid: " << command->cid() << ", "
        << "Sid: " << command->sid() << ", "
        << "Source: " << command->source() << ", "
        << "Destination: " << command->destination() << ", ";
    command->dump(o);
    o << "}";
    o << std::dec;
    return o;
}

std::ostream& operator<<(std::ostream& o, const ib::Command* command)
{
    if (command == NULL) {
        o << "Command: NULL";
        return o;
    }
    ib::Command * c = (ib::Command *)command;
    o << c;
    return o;
}

