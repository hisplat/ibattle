#include "command_parser.h"

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

    Command::CommandType type = (Command::CommandType)header->type;
    Command::CommandAction action = (Command::CommandAction)header->action;
    int cid = header->cid;
    int sid = header->sid;
    switch (action) {
    case Command::eAction_Invoke:
    case Command::eAction_Ret:
    case Command::eAction_Ack:
    case Command::eAction_Heartbit:
    case Command::eAction_Game:
    case Command::eAction_Play:
    case Command::eAction_Control:
    case Command::eAction_Live:
    case Command::eAction_Database:
    default:
        break;
    }

    return NULL;
}


int makePacket(base::Buffer& packet)
{
    return 0;
}


} // namespace CommandParser
} // namespace ib

