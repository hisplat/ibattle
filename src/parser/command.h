
#pragma once

#include "base/helper.h"

namespace ib {

class Command {
public:
    Command() {}
    virtual ~Command() {}
    DECLARE_CLASS(Command);
};

} // namespace ib 

