
#pragma once
#include "command.h"
#include "base/buffer.h"

namespace ib {
class Parser {
public:
    Parser();
    virtual ~Parser();


    // > 0: success, length of data. should try parse(NULL, 0, ...) again.
    // = 0: unknown bytes erased, try again.
    // < 0: nothing parsed. need more data.
    int parse(const void * moredata = NULL, int len = 0, bool check = true);
    virtual void onPacketParsed(const void* data, int len) = 0;

    // void createPacket(Command* command);
    // virtual void onPacketCreated(const void * data, int len) = 0;


private:
    base::Buffer mBuffer;

    typedef enum {
        eParseState_Init,
        eParseState_MagicReceived,
        eParseState_HeaderReceived,
        eParseState_BodyReceived,
    } ParseState;

    ParseState mParseState;
};

}  // namespace ib 

