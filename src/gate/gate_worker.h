
#pragma once

#include "common/worker.h"
#include "parser/parser_worker.h"
#include <string>

namespace ib {

class GateWorker : public ParserWorker {
public:
    GateWorker(Worker::Device* device);
    virtual ~GateWorker();
    DECLARE_CLASS(GateWorker);

    typedef enum {
        eType_Server,
        eType_Node,
        eType_Watcher,
        eType_Controller,
        eType_Database,
    } WorkerType;
    void setType(WorkerType type) { mType = type; }
    WorkerType type() { return mType; }

    void setUuid(const std::string& uuid) { mUuid = uuid; }
    void setName(const std::string& name) { mName = name; }
    
    const std::string& uuid() { return mUuid; }
    const std::string& name() { return mName; }

protected:
    GateWorker();

private:
    WorkerType mType;
    std::string mUuid;
    std::string mName;
};
} // namespace ib

