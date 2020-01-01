#include "command_parser.h"

#include "login_command.h"
#include "heartbit_command.h"

namespace ib {

namespace CommandParser {

typedef struct {
    __int32_t type;
    __int32_t action;
    __int32_t cid;
    __int32_t sid;
    char source[32];
    char destination[32];
} __attribute__((packed)) CommandHeader;

Command * makeCommand(const void * data, int len)
{
    CommandHeader * header = (CommandHeader*)data;
    const char * p = NULL;
    if (len > (int)sizeof(CommandHeader)) {
        p = (const char *)data;
        p += sizeof(CommandHeader);
        len -= sizeof(CommandHeader);
    } else {
        len = 0;
    }

    Command * command = NULL;
    Command::CommandType type = (Command::CommandType)header->type;
    Command::CommandAction action = (Command::CommandAction)header->action;
    int cid = header->cid;
    int sid = header->sid;

    switch (action) {
    case Command::eAction_Invoke:
    case Command::eAction_Ret:
    case Command::eAction_Ack:
        break;
    case Command::eAction_Heartbit:
        command = new HeartbitCommand();
        break;
    case Command::eAction_Game:
    case Command::eAction_Play:
    case Command::eAction_Control:
    case Command::eAction_Live:
    case Command::eAction_Database:
        break;
    case Command::eAction_Login:
        command = new LoginCommand();
        break;
    default:
        break;
    }

    if (command != NULL) {
        command->setType(type);
        command->setAction(action);
        command->setCid(cid);
        command->setSid(sid);
        command->setSource(header->source);
        command->setDestination(header->destination);

        base::Buffer buf(p, len);
        command->loadFrom(buf);
    }

    return command;
}


base::Buffer packCommand(Command* command)
{
    base::Buffer buf;

    CommandHeader header;
    memset(&header, 0, sizeof(header));

    header.cid = command->cid();
    header.sid = command->sid();
    header.type = command->type();
    header.action = command->action();
    snprintf(header.source, sizeof(header.source), "%s", command->source().c_str());
    snprintf(header.destination, sizeof(header.destination), "%s", command->destination().c_str());

    buf.append((const char*)&header, sizeof(header));

    // serialize to data.
    base::Buffer data;
    command->saveTo(data);

    buf.append(data);
    return buf;
}


} // namespace CommandParser
} // namespace ib

