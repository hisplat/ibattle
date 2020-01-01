
#pragma once
#include "command.h"
#include <string>

namespace ib {

class LoginCommand : public Command {
public:
    LoginCommand() : Command(Command::eType_Sync, Command::eAction_Login) {}
    virtual ~LoginCommand() {}
    DECLARE_CLASS(LoginCommand);

    void setToken(const std::string& token) { mToken = token; }
    const std::string& token() { return mToken; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mToken; }
    virtual void Deserialize(base::Archive& ar) { ar >> mToken; }

    std::string mToken;

private:
    LoginCommand(CommandType type, CommandAction action) : Command(type, action) {}

};
} // namespace ib


