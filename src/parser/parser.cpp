#include "parser.h"
#include "base/helper.h"
#include "base/logging.h"

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

// void Parser::createPacket(Command* command)
// {
//     PacketHeader header;
//     memset(&header, 0, sizeof(header));
// 
//     Buffer packet;
//     int len = command->createPacket(packet);
// 
//     memcpy(header.magic, "HiRPC", 5);
//     header.version = 1;
//     header.hdrSize = sizeof(header);
//     header.dataSize = len; // command->getHeaderLength() + len; // command->getData().length();
// 
//     unsigned int crc1 = helper::crc32(&header, sizeof(header) - 16);
//     unsigned int crc2 = helper::crc32(packet.buffer(), packet.length());
// 
//     memcpy(header.crc, &crc1, sizeof(crc1));
//     memcpy(header.crc + 8, &crc2, sizeof(crc2));
// 
//     Buffer buffer;
//     buffer.append((const char *)&header, sizeof(header));
//     buffer.append(packet);
// 
//     onPacketCreated(buffer.buffer(), buffer.length());
// }


} // namespace ib

