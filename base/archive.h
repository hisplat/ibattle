#pragma once

#include <string>
#include <list>
#include "buffer.h"
#include "logging.h"

namespace base {

class Archive;
class Serializer {
public:
    Serializer() {}
    virtual ~Serializer() {}

    void loadFrom(const Buffer& buf);
    void saveTo(Buffer& buf);

protected:
    friend class Archive;
    virtual void Serialize(Archive& ar) = 0;
    virtual void Deserialize(Archive& ar) = 0;
};

template <typename T> class isSerializer {
    class No {};
    class Yes { No _no[2]; };
    static No check(...);
    static Yes check(const Serializer*);
public:
    operator bool() {
        return sizeof(check((T*)((void*)(0)))) == sizeof(Yes);
    }
};

class Archive {
public:
    Archive();
    Archive(const void* data, int len);
    ~Archive();

    template <class T>
    Archive& operator<<(const T& t) {
        if (isSerializer<T>()) {
            Serializer* s = (Serializer*)&t;
            s->Serialize(*this);
        } else {
            put(&t, sizeof(T));
        }
        return *this;
    }

    template <class T>
    Archive& operator>>(T& t) {
        if (isSerializer<T>()) {
            Serializer* s = (Serializer*)&t;
            s->Deserialize(*this);
        } else {
            get(&t, sizeof(T));
        }
        return *this;
    }
    
    Archive& operator<<(const std::string& s);
    Archive& operator>>(std::string& s);
    Archive& operator<<(const Buffer& buf);
    Archive& operator>>(Buffer& buf);

    void copyTo(Buffer& buf);

private:
    void put(const void* data, int len);
    void get(void* data, int len);
    void get(Buffer& buf);

    std::list<Buffer *> mDataList;
};


} // namespace base

