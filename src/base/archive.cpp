#include "archive.h"
#include "logging.h"

namespace base {

void Serializer::loadFrom(const Buffer& buf)
{
    Archive ar(buf.buffer(), buf.length());
    Deserialize(ar);
}

void Serializer::saveTo(Buffer& buf)
{
    Archive ar;
    Serialize(ar);
    ar.copyTo(buf);
}

Archive::Archive()
{
}

Archive::Archive(const void* data, int len)
{
    const char * p = (const char *)data;
    while (len > 4) {
        __int32_t l = *((__int32_t*)p);
        p += 4;
        put(p, l);
        p += l;
        len = len - 4 - l;
    }
}

Archive::~Archive()
{
    for (std::list<Buffer*>::iterator it = mDataList.begin(); it != mDataList.end(); it++) {
        Buffer* b = *it;
        delete b;
    }
}

void Archive::put(const void* data, int len)
{
    mDataList.push_back(new Buffer((const char*)data, len));
}

void Archive::get(void* data, int len)
{
    // LCHECK(!mDataList.empty()) << "no data available.";
    if (mDataList.empty()) {
        return;
    }
    Buffer * buf = mDataList.front();
    mDataList.pop_front();
    if (len > buf->length()) {
        len = buf->length();
    }
    memcpy(data, buf->buffer(), len);
    delete buf;
}

void Archive::get(Buffer& buf)
{
    // LCHECK(!mDataList.empty()) << "no data available.";
    if (mDataList.empty()) {
        return;
    }
    Buffer * front = mDataList.front();
    mDataList.pop_front();
    buf.assign(front->buffer(), front->length());
    delete front;
}

Archive& Archive::operator<<(const std::string& s)
{
    put(s.c_str(), s.length() + 1);
    return *this;
}

Archive& Archive::operator>>(std::string& s)
{
    Buffer b;
    get(b);
    // null can not be assigned to std::string
    if (b.buffer() != NULL) {
        s = b.buffer();
    }
    return *this;
}

Archive& Archive::operator<<(const Buffer& buf)
{
    mDataList.push_back(new Buffer(buf.buffer(), buf.length()));
    return *this;
}

Archive& Archive::operator>>(Buffer& buf)
{
    get(buf);
    return *this;
}

void Archive::copyTo(Buffer& buf)
{
    buf.clear();
    for (std::list<Buffer*>::iterator it = mDataList.begin(); it != mDataList.end(); it++) {
        Buffer* b = *it;
        __int32_t len = b->length();
        buf.append((const char *)&len, 4);
        buf.append(b->buffer(), b->length());
    }
}

} // namespace base


