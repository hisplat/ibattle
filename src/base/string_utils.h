#pragma once

#include <list>
#include <vector>
#include <string>


namespace base {
namespace helper {

std::string trim(const std::string& input, const char * trim_chars = " \r\n\t");
int split(const std::string& input, std::list<std::string>& output, char delimiter = ' ');
int split(const std::string& input, std::vector<std::string>& output, char delimiter = ' ');
std::vector<std::string> split(const std::string& input, char delimiter = ' ');
bool startsWith(const std::string& s, const std::string& n);
std::string implode(std::vector<std::string> v, const char * delimiter = ",");
std::string intToString(int input);

} // namespace helper
} // namespace base 

