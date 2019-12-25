
#pragma once

#include "base/helper.h"
#include "base/archive.h"
#include <string>

namespace ib {

class Command : public base::Serializer {
public:
    typedef enum {
        eType_Async,
        eType_Sync,
    } CommandType;

    typedef enum {
        eAction_Invoke,
        eAction_Ret,
        eAction_Ack,
        eAction_Heartbit,

        eAction_Game,
        eAction_Play,
        eAction_Control,
        eAction_Live,
        eAction_Database,

    } CommandAction;

    Command();
    Command(CommandType type, CommandAction action);
    virtual ~Command();
    DECLARE_CLASS(Command);


    CommandType type() { return mType; }
    CommandAction action() { return mAction; }
    int cid() { return mCid; }
    int sid() { return mSid; }
    std::string source() { return mSource; }
    std::string destination() { return mDestination; }


    virtual void dump(std::ostream& o) {}
protected:
    virtual void Serialize(base::Archive& ar) {}
    virtual void Deserialize(base::Archive& ar) {}

private:
    int mCid;
    int mSid;
    CommandType mType;
    CommandAction mAction;
    std::string mDestination;
    std::string mSource;
};

} // namespace ib 

std::ostream& operator<<(std::ostream& o, const ib::Command* command);
std::ostream& operator<<(std::ostream& o, ib::Command* command);


