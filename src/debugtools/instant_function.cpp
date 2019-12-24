
#include "instant_function.h"
#include "base/logging.h"

namespace ib {
namespace debugtools {

InstantFunction* InstantFunction::mFunctions[kMaxFunctions] = {0};
static int gFunctions = 0;

InstantFunction::InstantFunction()
{
    InstantFunction ** p = mFunctions;
    int count = kMaxFunctions;
    while (*p != NULL && count > 0) {
        p++;
        count--;
    }
    if (count <= 0) {
        FATAL() <<  "No more spaces for instant functions. you should enlarge MAX_FUNCTIONS.";
    }
    *p = this;
    gFunctions++;
    mFunctions[gFunctions] = NULL;
}

InstantFunction::~InstantFunction()
{
}

} // namespace debugtools
} // namespace ib

