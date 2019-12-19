
#include "helper.h"
#include "logging.h"
#include "instant_function.h"

namespace {


INSTANT_FUNCTION(netinfo)
{
    std::vector<base::helper::NetDeviceInfo> r = base::helper::getNetworkDevices();

    for (int i = 0; i < (int)r.size(); i++) {
        NFLOG() << r[i].iface << ": " << r[i].address << ", " << r[i].mask << ", " << r[i].broadcast;
    }
    return 0;
}

INSTANT_FUNCTION(uuid)
{
    std::string uuid1;
    std::string uuid2;
    {
        ScopeTrace(uuid1);
        uuid1 = base::helper::uuid();
    }
    {
        ScopeTrace(uuid2);
        uuid2 = base::helper::uuid2();
    }
    fprintf(stderr, "%s\n", uuid1.c_str());
    fprintf(stderr, "%s\n", uuid2.c_str());
    return 0;
}

}

