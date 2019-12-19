#pragma once


#include <iostream>

namespace base {

class Buffer {
public:
    Buffer(const void* buffer, int size);
    Buffer(int size, char c = '\0');
    Buffer(const std::string& s);
    Buffer(const char * str);
    Buffer();

protected:
    Buffer(const Buffer& o);

public:
    Buffer(const Buffer&& o);
    ~Buffer();

    int length() const { return len; }
    int str_length();
    const char* buffer() const { return data; }
    const char * offset(int offset) const { return data + offset; }
    bool isEmpty();

    void assign(const void* data, int len);
    void assign(int size, char c = '\0');
    void assign(const char * str);
    void assign(const std::string& str);
    void clear() { assign(0); }

    void append(const void* data, int size);
    void append(const Buffer& o);
    void append(const std::string& str);
    void append(const char * str);
    void erase_before(int offset);

    char& at(int offset);
    char& operator[](int offset);
    char operator[] (int offset) const;
    operator char*() { return data; }
    operator const char*() { return data; }
    operator unsigned char*() { return (unsigned char*)data; }
    operator void*() { return data; }
    Buffer& operator=(const Buffer& o);
    Buffer& operator=(const Buffer* o);
    unsigned char* operator+(int offset) { return (unsigned char*)data + offset; }
    bool operator==(const std::string& s) const;

    void copy(const Buffer& o);
    void move(const Buffer& o);

    int     mArgN;
    long    mArgL;
    void*   mArgP;

private:
    int     len;
    char*   data;
};

} // namespace base 


std::ostream& operator<<(std::ostream& o, const base::Buffer& ub);



