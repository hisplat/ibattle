#include "parser.h"
#include "base/helper.h"
#include "base/logging.h"
#include "parser/command_parser.h"

#include <string.h>

namespace ib {

typedef struct {
    char magic[4];
    __int32_t dataSize;
} __attribute__ ((packed)) PacketHeader;

Parser::Parser()
    : mParseState(eParseState_Init)
{
}

Parser::~Parser()
{
}

// ret:
//      > 0: packet found, try again.
//      = 0: packet not found, try again.
//      < 0: packet not found, need more data.
int Parser::parse(const void * moredata, int len, bool check)
{
    if (moredata != NULL && len > 0) {
        mBuffer.append(moredata, len);
    }

    PacketHeader * header = (PacketHeader*)mBuffer.buffer();
    if (mParseState == eParseState_Init) {
        if (mBuffer.length() < 4) {
            return -1;
        }

        if (memcmp(header->magic, "iBt!", 4) == 0) {
            mParseState = eParseState_MagicReceived;
        } else {
            VERBOSE() << "unexpected packet (illegal magic). ignore first 1 byte and try again.";
            mBuffer.erase_before(1);
            return 0;
        }
    } else if (mParseState == eParseState_MagicReceived) {
        if (mBuffer.length() < (int)sizeof(PacketHeader)) {
            return -1;
        }
        mParseState = eParseState_HeaderReceived;
    } else if (mParseState == eParseState_HeaderReceived) {
       int datalen = sizeof(PacketHeader) + header->dataSize;
        // if need more buf.
        if (mBuffer.length() < datalen) {
            return -1;
        }
        mParseState = eParseState_BodyReceived;
    } else if (mParseState == eParseState_BodyReceived) {
        int datalen = sizeof(PacketHeader) + header->dataSize;
        int offset = sizeof(PacketHeader);
        onPacketParsed(mBuffer.buffer(), datalen, sizeof(PacketHeader));
        mBuffer.erase_before(datalen);
        mParseState = eParseState_Init;
        return datalen;
    } else {
        mParseState = eParseState_Init;
        FATAL() << "Should never run here.";
    }
    return 0;
}

base::Buffer Parser::createPacket(Command* command)
{
    PacketHeader header;
    memset(&header, 0, sizeof(header));

    base::Buffer cmdbuf = CommandParser::packCommand(command);
    int len = cmdbuf.length();

    memcpy(header.magic, "iBt!", 4);
    header.dataSize = len;

    base::Buffer buffer;
    buffer.append((const char *)&header, sizeof(header));
    buffer.append(cmdbuf);
    return buffer;
}


} // namespace ib

