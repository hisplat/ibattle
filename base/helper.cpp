#include "helper.h"

#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "logging.h"


// for ifaddrs
#include <ifaddrs.h>
#include <arpa/inet.h>

namespace base {
namespace helper {

std::string trim(const std::string& input, const char * trim_chars)
{
    size_t  pos1 = input.find_first_not_of(trim_chars);
    size_t  pos2 = input.find_last_not_of(trim_chars);
    if (input.empty() || pos1 == std::string::npos || pos2 == std::string::npos)
        return std::string("");
    return input.substr(pos1, pos2 - pos1 + 1);
}

int split(const std::string& input, std::list<std::string>& output, char delimiter)
{
    std::stringstream   oss;
    oss << input;
    std::string word;
    output.clear();
    while (std::getline(oss, word, delimiter)) {
        output.push_back(word);
    }
    return output.size();
}

int split(const std::string& input, std::vector<std::string>& output, char delimiter)
{
    std::stringstream   oss;
    oss << input;
    std::string word;
    output.clear();
    while (std::getline(oss, word, delimiter)) {
        output.push_back(word);
    }
    return output.size();
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> v;
    std::string::size_type pos1, pos2;
    pos2 = s.find(delimiter);
    pos1 = 0;
    while (pos2 != std::string::npos) {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
        pos2 = s.find(delimiter, pos1);
    }
    v.push_back(s.substr(pos1));
    return v;
}

std::string intToString(int input)
{
    char l_temp[40];
    sprintf(l_temp, "%d", input);
    return std::string(l_temp);
}

bool startsWith(const std::string& s, const std::string& n)
{
    if (s.length() < n.length()) {
        return false;
    }
    if (n.length() == 0) {
        return false;
    }
    return s.substr(0, n.length()) == n;
}

std::string implode(std::vector<std::string> v, const char * delimiter)
{
    std::string result;
    for (int i = 0; i < (int)v.size(); i++) {
        if (i != 0) {
            result += delimiter;
        }
        result += v[i];
    }
    return result;
}


long mtick()
{
    struct timespec ts;
    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long n = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return n;
}

long long ntick()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (long long)(t.tv_sec)*1000000000LL + t.tv_nsec;
}

const std::string& getCurrentProcessName()
{
    static std::string gCurrentProcessName;
    if (gCurrentProcessName.length() > 0) {
        return gCurrentProcessName;
    }
    char buffer[1024] = {0};
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0) {
        PERROR() << "open";
        return gCurrentProcessName;
    }
    ssize_t ret = read(fd, buffer, sizeof(buffer) - 1);
    if (ret >= 0) {
        buffer[ret] = '\0';
    }
    close(fd);
    gCurrentProcessName = buffer;
    return gCurrentProcessName;
}

int filesize(const std::string& file)
{
    struct stat buf;
    int ret = stat(file.c_str(), &buf);
    if (ret != 0) {
        PERROR() << "stat fail.";
        return -errno;
    }
    return (int)buf.st_size;
}

bool fileexists(const std::string& file)
{
    int ret = access(file.c_str(), F_OK | R_OK);
    return (ret == 0);
}

std::vector<NetDeviceInfo> getNetworkDevices()
{
    struct ifaddrs * ifa = NULL;
    struct ifaddrs * iflist = NULL;

    std::vector<NetDeviceInfo> result;
    if (getifaddrs(&iflist) < 0) {
        PERROR() << "getifaddrs.";
        return result;
    }
    for (ifa = iflist; ifa != NULL; ifa = ifa->ifa_next) {
        // RUN_HERE() << "FAMILY: " << ifa->ifa_addr->sa_family;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            NetDeviceInfo info;

            info.iface = ifa->ifa_name;

            struct sockaddr_in * sin = (struct sockaddr_in *)ifa->ifa_addr;
            info.address = inet_ntoa(sin->sin_addr);

            sin = (struct sockaddr_in *)ifa->ifa_broadaddr;
            info.broadcast = inet_ntoa(sin->sin_addr);
            if (info.broadcast == "127.0.0.1") {
                info.broadcast = "127.255.255.255";
            }

            sin = (struct sockaddr_in *)ifa->ifa_netmask;
            info.mask = inet_ntoa(sin->sin_addr);

            result.push_back(info);
        }
    }
    freeifaddrs(iflist);
    return result;
}

std::vector<std::string> getInterfaces()
{
    std::vector<std::string> v;
    std::vector<NetDeviceInfo> infos = getNetworkDevices();
    for (int i = 0; i < (int)infos.size(); i++) {
        NetDeviceInfo& info = infos[i];
        v.push_back(info.iface);
    }
    return v;
}

std::string uuid2()
{
    ATTENTION();
    char buf[37];
    char * p = buf;
    const char * c = "89ab";
    unsigned int seed = (int)((ntick() % INT_MAX) & 0xffffffff);
    srand(seed);
    for (int n = 0; n < 16; n++) {
        int b = rand() % 255;
        switch (n) {
        case 6:
            sprintf(p, "4%x", b % 15);
            break;
        case 8:
            sprintf(p, "%c%x", c[rand() % strlen(c)], b % 15);
            break;
        default:
            sprintf(p, "%02x", b);
            break;
        }
        p += 2;
        switch (n) {
        case 3:
        case 5:
        case 7:
        case 9:
            *p = '-';
            p++;
            break;
        }
    }
    *p = '\0';
    return buf;
}

std::string uuid()
{
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        return uuid2();
    }
    char result[37] = {0};
    char buf[16];
    int total = 0;
    while (total < 16) {
        int ret = read(fd, buf + total, 16 - total);
        if (ret <= 0) {
            close(fd);
            return uuid2();
        }
        total += ret;
    }
    close(fd);

    sprintf(result, "%02x%02x%02x%02x", buf[0] & 0xff, buf[1] & 0xff, buf[2] & 0xff, buf[3] & 0xff);
    sprintf(result + 9, "%02x%02x", buf[4] & 0xff, buf[5] & 0xff);
    sprintf(result + 14, "%02x%02x", buf[6] & 0xff, buf[7] & 0xff);
    sprintf(result + 19, "%02x%02x", buf[8] & 0xff, buf[9] & 0xff);
    sprintf(result + 24, "%02x%02x%02x%02x%02x%02x", buf[10] & 0xff, buf[11] & 0xff, buf[12] & 0xff, buf[13] & 0xff, buf[14] & 0xff, buf[15] & 0xff);
    result[8] = '-';
    result[13] = '-';
    result[18] = '-';
    result[23] = '-';
    result[36] = '\0';
    return result;
}

} // namespace helper
} // namespace base

