
#pragma once

#include <string>

namespace ib {
namespace debugtools {

class InstantFunction {
public:
    InstantFunction();
    virtual ~InstantFunction();

    virtual std::string getFunctionName() = 0;
    virtual int run(int argc, char * argv[]) = 0;

public:
    static const int kMaxFunctions = 20;
    static InstantFunction * mFunctions[kMaxFunctions];
};

} // namespace debugtools
} // namespace ib

#define INSTANT_FUNCTION(C) \
    class C : public ib::debugtools::InstantFunction { \
        virtual std::string getFunctionName() { return #C; } \
        virtual int run(int argc, char * argv[]); \
    }; \
static C _temp_ ## C; \
int C::run(int argc, char * argv[])

