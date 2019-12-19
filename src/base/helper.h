#pragma once

#include <list>
#include <vector>
#include <string>

namespace base {
namespace helper {

std::string trim(const std::string& input, const char * trim_chars = " \r\n\t");

// split
int split(const std::string& input, std::list<std::string>& output, char delimiter = ' ');
int split(const std::string& input, std::vector<std::string>& output, char delimiter = ' ');
std::vector<std::string> split(const std::string& input, char delimiter = ' ');
bool startsWith(const std::string& s, const std::string& n);
std::string implode(std::vector<std::string> v, const char * delimiter = ",");

// int to string
std::string intToString(int input);


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

