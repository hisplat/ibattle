
#pragma once
#include "command.h"
#include <string>

namespace ib {

class ReturnCommand : public Command {
public:
    ReturnCommand() : Command(Command::eType_Async, Command::eAction_Ret) {}
    virtual ~ReturnCommand() {}
    DECLARE_CLASS(ReturnCommand);

    void setData(const std::string& b) { mData = b; }
    const std::string& data() { return mData; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mData; }
    virtual void Deserialize(base::Archive& ar) { ar >> mData; }

private:
    ReturnCommand(CommandType type, CommandAction action) : Command(type, action) {}
    std::string mData;
};
} // namespace ib


