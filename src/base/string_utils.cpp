#include "string_utils.h"

#include <sstream>
#include <algorithm>
#include <string.h>
#include <errno.h>


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

} // namespace helper
} // namespace base

