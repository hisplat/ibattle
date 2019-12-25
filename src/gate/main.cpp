
#include "gate.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "base/logging.h"


static ib::Gate * gate = NULL;

static void usage()
{
    fprintf(stderr, "Usage: gate [OPTIONS]\n");
    fprintf(stderr, "   -s, --server-port=PORT          server port.\n");
    fprintf(stderr, "   -n, --node-port=PORT            node port.\n");
    fprintf(stderr, "   -w, --watcher-port=PORT         watcher port.\n");
    fprintf(stderr, "   -c, --controller-port=PORT      controller port.\n");
    fprintf(stderr, "   -b, --database-port=PORT        database port.\n");
    fprintf(stderr, "   -v, --verbose                   chatty mode.\n");
    fprintf(stderr, "   -q, --quiet                     quiet mode.\n");
    fprintf(stderr, "   -h, --help                      display this screen and quit.\n");
    fprintf(stderr, "\n");
}

static void init_signal();
static void nf_signal_handler(int signal_number, siginfo_t* info, void*)
{
    signal(signal_number, SIG_DFL);

    VERBOSE() << "received signal: " << signal_number;

    {
        ::logging::LogMessage msg("iBattle", ::logging::kLogLevel_Important);
        gate->dump(msg.stream());
    }

    switch (signal_number) {
    case SIGINT:
        gate->stop();
        break;
    case SIGUSR1:
    case SIGUSR2:
    case SIGQUIT:
    case SIGPIPE:
    default:
        kill(getpid(), signal_number);
        break;
    }
}

static void init_signal()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));

    sigemptyset(&action.sa_mask);
    action.sa_sigaction = nf_signal_handler;
    action.sa_flags = SA_RESTART | SA_SIGINFO;

    action.sa_flags |= SA_ONSTACK;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGPIPE, &action, NULL);
}


int main(int argc, char* argv[])
{
    int serverport = 20100;
    int nodeport = 20101;
    int controllerport = 20102;
    int watcherport = 20103;
    int databaseport = 10204;
    bool verbose = false;
    bool quiet = false;

    int c;
    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            {"server-port",         required_argument, 0, 's'},
            {"node-port",           required_argument, 0, 'n'},
            {"watcher-port",        required_argument, 0, 'w'},
            {"controller-port",     required_argument, 0, 'c'},
            {"database-port",       required_argument, 0, 'b'},
            {"verbose",             no_argument,       0, 'v'},
            {"quiet",               no_argument,       0, 'q'},
            {"help",                no_argument,       0, 'h'},
            {0,                     0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "s:n:w:c:b:vqh", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 's':
            serverport = atoi(optarg);
            break;
        case 'n':
            nodeport = atoi(optarg);
            break;
        case 'w':
            watcherport = atoi(optarg);
            break;
        case 'c':
            controllerport = atoi(optarg);
            break;
        case 'b':
            databaseport = atoi(optarg);
            break;

        case 'v':
            verbose = true;
            break;
        case 'q':
            quiet = true;
            break;
        case 'h':
            usage();
            return 0;
        default:
            break;
        }
    }
    if (verbose && quiet) {
        FATAL() << "should not run under both verbose and quiet mode.";
    } else if (verbose) {
        logging::setLogLevel(logging::kLogLevel_Verbose);
    } else if (quiet) {
        logging::setLogLevel(logging::kLogLevel_Quiet);
    } else {
        logging::setLogLevel(logging::kLogLevel_Important);
    }

    // init_signal();
    gate = new ib::Gate();
    gate->run(serverport, nodeport, controllerport, watcherport, databaseport);
    delete gate;
    return 0;
}

