
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include "base/helper.h"
#include "base/logging.h"
#include "instant_function.h"

static void usage()
{
    SLOG() << "Usage: debugtools [OPTION]";
    SLOG() << "    -l, --list              list instant functions.";
    SLOG() << "    -w, --window            windowmode.";
    SLOG() << "    -h, --help              display this help and exit.";
    SLOG() << "";
    SLOG() << "Instant Functions:";
    
    for (ib::debugtools::InstantFunction ** func = ib::debugtools::InstantFunction::mFunctions; *func != NULL; func++) {
        SLOG() << "Usage: debugtools " << (*func)->getFunctionName() << " [OPTION]";
    }

    SLOG() << "";
}

static int list_instant_functions()
{
    for (ib::debugtools::InstantFunction ** func = ib::debugtools::InstantFunction::mFunctions; *func != NULL; func++) {
        fprintf(stderr, "%s\n", (*func)->getFunctionName().c_str());
    }
    return 1;
}

int main(int argc, char* argv[])
{
    // log cmdlines into logd for debugging.
    std::vector<std::string> args(argv, argv + argc);
    NFLOG() << "starting debugtools with command: '" << base::helper::implode(args, " ") << "', PPID: " << getppid() << ", UID: " << getuid();

    // instant functions.
    if (argc > 1) {
        for (ib::debugtools::InstantFunction ** func = ib::debugtools::InstantFunction::mFunctions; *func != NULL; func++) {
            if ((*func)->getFunctionName() == argv[1]) {
                NFLOG() << "instant function: " << argv[1];
                return (*func)->run(argc, argv);
            }
        }
    }

    return 0;
}

