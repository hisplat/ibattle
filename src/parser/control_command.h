
#pragma once
#include "command.h"
#include <string>

namespace ib {

class ControlCommand : public Command {
public:
    ControlCommand() : Command(Command::eType_Async, Command::eAction_Control) {}
    virtual ~ControlCommand() {}

    DECLARE_CLASS(ControlCommand);

    const std::string& command() { return mCommand; }
    void setCommand(const std::string& cmd) { mCommand = cmd; }
    virtual void dump(std::ostream& o) { o << "Command: \"" << mCommand << "\""; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mCommand; }
    virtual void Deserialize(base::Archive& ar) { ar >> mCommand; }

    std::string mCommand;

private:
    ControlCommand(CommandType type, CommandAction action) : Command(type, action) {}
};
} // namespace ib


