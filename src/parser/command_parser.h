
#pragma once

#include "command.h"
#include "base/buffer.h"

namespace ib {

namespace CommandParser {

Command * makeCommand(const void * data, int len);

int makePacket(base::Buffer& packet);

} // namespace CommandParser
} // namespace ib

