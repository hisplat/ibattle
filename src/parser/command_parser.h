
#pragma once

#include "command.h"
#include "base/buffer.h"

namespace ib {

namespace CommandParser {

Command * makeCommand(const void * data, int len);

base::Buffer packCommand(Command* command);

} // namespace CommandParser
} // namespace ib

