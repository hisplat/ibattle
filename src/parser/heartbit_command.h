
#pragma once
#include "command.h"

namespace ib {

class HeartbitCommand : public Command {
public:
    HeartbitCommand() : Command(Command::eType_Async, Command::eAction_Heartbit) {}
    virtual ~HeartbitCommand() {}

    DECLARE_CLASS(HeartbitCommand);

protected:
    virtual void Serialize(base::Archive& ar) {}
    virtual void Deserialize(base::Archive& ar) {}

private:
    HeartbitCommand(CommandType type, CommandAction action) : Command(type, action) {}
};
} // namespace ib


