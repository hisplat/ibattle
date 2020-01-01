
#pragma once
#include "command.h"
#include <string>

namespace ib {

class InvokeCommand : public Command {
public:
    InvokeCommand() : Command(Command::eType_Async, Command::eAction_Invoke) {}
    virtual ~InvokeCommand() {}

    DECLARE_CLASS(InvokeCommand);

    void setData(const std::string& b) { mData = b; }
    const std::string& data() { return mData; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mData; }
    virtual void Deserialize(base::Archive& ar) { ar >> mData; }

private:
    InvokeCommand(CommandType type, CommandAction action) : Command(type, action) {}

    std::string mData;
};
} // namespace ib


