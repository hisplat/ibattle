
#pragma once
#include <iostream>
#include <map>
#include <string>

#define REGISTER_MESSAGE(what)  base::MessageHelper::registerMessage(what, #what)

namespace base {

class Message {
public:
    int what;
    int arg1;
    int arg2;
    int arg3;
    void * obj;

    long when;
    class Callback {
    public:
        virtual ~Callback() {}
        virtual void onMessage(Message message) = 0;
    };
    Callback* callback;

    Message();

    static Message obtain();
    static Message obtain(int what, int arg1 = 0, int arg2 = 0, void* obj = 0);
    static Message obtain(int what, int arg1, int arg2, int arg3, void* obj);
    static Message obtain(Callback* callback, int what, int arg1 = 0, int arg2 = 0, void* obj = 0);
};

class MessageHelper {
public:
    static void registerMessage(int what, const char * name);
    static const char * name(int what);
private:
    static std::map<int, std::string> mNames;
};
} // namespace base;

std::ostream& operator<<(std::ostream& o, base::Message& msg);

