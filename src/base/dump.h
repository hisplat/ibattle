
#pragma once

#include "logging.h"

namespace base {

void dump(const void * data, int len, const char * file = "", const char * func = "", int line = 0, const char * tag = "base:dump");

}


#define NFDUMP(data, len) ::base::dump(data, len, __FILE__, __FUNC__, __LINE__)

