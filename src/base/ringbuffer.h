
#pragma once

namespace base {

template <class T>
class RingBuffer {
public:
    RingBuffer(int size) : mSize(size), mBegin(0), mLength(0) {
        mBuffer = new T[size];
    };

    ~RingBuffer() {
        delete [] mBuffer;
    }

    int length() {
        return mLength;
    }

    int size() {
        return mSize;
    }

    void append(const T& t) {
        if (mLength >= mSize) {
            mLength = mSize;
            mBuffer[mBegin] = t;
            mBegin++;
            if (mBegin >= mSize) {
                mBegin = 0;
            }
        } else {
            int next = mBegin + mLength;
            if (next >= mSize) {
                next -= mSize;
            }
            mBuffer[next] = t;
            mLength++;
        }
    }

    T& at(int offset) {
        if (offset >= 0) {
            int pos = mBegin + offset;
            while (pos >= mLength) {
                pos -= mLength;
            }
            return mBuffer[pos];
        } else {
            int pos = mBegin + mLength + offset;
            while (pos < mBegin) {
                pos += mLength;
            }
            return mBuffer[pos];
        }
    }

    T& operator[](int offset) {
        return at(offset);
    }
private:
    RingBuffer() = delete;

    T* mBuffer;
    int mSize;
    int mBegin;
    int mLength;
};

} // namespace base

