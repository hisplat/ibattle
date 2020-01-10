#pragma once

#include "string_utils.h"
#include <list>
#include <vector>
#include <string>
#include "logging.h"

#define DECLARE_CLASS(clz) \
    virtual const char * getClassName() { return #clz; } \
    virtual bool isClass(const std::string& classname) { return classname == #clz; }

namespace base {
namespace helper {


long mtick();
long long ntick();

const std::string& getCurrentProcessName();

std::string uuid();
std::string uuid2();

std::string md5sum(const std::string& file);
unsigned int crc32(const char * filename);
unsigned int crc32(const void * buffer, int len);

int filesize(const std::string& file);
bool fileexists(const std::string& file);
bool isdir(const std::string& file);


// net
class NetDeviceInfo {
public:
    std::string iface;
    std::string address;
    std::string mask;
    std::string broadcast;
};
std::vector<NetDeviceInfo> getNetworkDevices();
std::vector<std::string> getInterfaces();


std::string getUnixSocketPath();
} // namespace helper
} // namespace base 

