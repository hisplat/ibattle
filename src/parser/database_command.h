
#pragma once
#include "command.h"
#include <string>

namespace ib {

class DatabaseCommand : public Command {
public:
    DatabaseCommand() : Command(Command::eType_Sync, Command::eAction_Database) {}
    virtual ~DatabaseCommand() {}
    DECLARE_CLASS(DatabaseCommand);

    void setToken(const std::string& token);
    const std::string& token() { return mToken; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mToken; }
    virtual void Deserialize(base::Archive& ar) { ar >> mToken; }

    std::string mToken;

private:
    DatabaseCommand(CommandType type, CommandAction action) : Command(type, action) {}

};
} // namespace ib


