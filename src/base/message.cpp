
#include "message.h"

namespace base {

Message::Message()
    : what(0)
    , arg1(0)
    , arg2(0)
    , arg3()
    , obj(0)
    , when(0)
    , callback(0)
{
}

Message Message::obtain()
{
    Message msg;
    return msg;
}

Message Message::obtain(int what, int arg1, int arg2, void* obj)
{
    Message msg;
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.obj = obj;
    return msg;
}

Message Message::obtain(int what, int arg1, int arg2, int arg3, void* obj)
{
    Message msg;
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.arg3 = arg3;
    msg.obj = obj;
    return msg;
}

Message Message::obtain(Callback* callback, int what, int arg1, int arg2, void* obj)
{
    Message msg;
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.obj = obj;
    msg.callback = callback;
    return msg;
}


std::map<int, std::string> MessageHelper::mNames;
void MessageHelper::registerMessage(int what, const char * name)
{
    mNames[what] = name;
}

const char * MessageHelper::name(int what)
{
    std::map<int, std::string>::iterator it = mNames.find(what);
    if (it == mNames.end()) {
        return NULL;
    }
    return it->second.c_str();
}

} // namespace base

std::ostream& operator<<(std::ostream& o, base::Message& msg)
{
    const char * name = base::MessageHelper::name(msg.what);
    if (name == NULL) {
        name = "(noname)";
    }

    o << "Message {"
      << "what: " << msg.what << ", "
      << "name: " << name << ", "
      << "arg1: " << msg.arg1 << ", "
      << "arg2: " << msg.arg2 << ", "
      << "arg3: " << msg.arg3 << ", "
      << "obj: " << msg.obj << ", "
      << "when: " << msg.when << ", "
      << "callback: " << msg.callback
      << "}";
    return o;
}
