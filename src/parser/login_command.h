
#pragma once
#include "command.h"
#include <string>

namespace ib {

class LoginCommand : public Command {
public:
    LoginCommand() : Command(Command::eType_Sync, Command::eAction_Login) {}
    virtual ~LoginCommand() {}
    DECLARE_CLASS(LoginCommand);

    void setUuid(const std::string& uuid) { mUuid = uuid; }
    void setName(const std::string& name) { mName = name; }

    const std::string& uuid() { return mUuid; }
    const std::string& name() { return mName; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mUuid << mName; }
    virtual void Deserialize(base::Archive& ar) { ar >> mUuid >> mName; }

    std::string mUuid;
    std::string mName;

private:
    LoginCommand(CommandType type, CommandAction action) : Command(type, action) {}

};
} // namespace ib


