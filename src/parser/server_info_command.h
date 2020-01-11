
#pragma once
#include "command.h"
#include <string>
#include <vector>
#include <map>

namespace ib {

class ServerInfoCommand : public Command {
public:
    ServerInfoCommand() : Command(Command::eType_Sync, Command::eAction_ServerInfo) {}
    virtual ~ServerInfoCommand() {}
    DECLARE_CLASS(ServerInfoCommand);

    typedef enum {
        eOp_SetPlayerCount = 0,
        eOp_AttachPlayers,
        eOp_Start,
        eOp_Score,
    } OpCode;


    OpCode op() { return mOp; }
    void setOp(OpCode op) { mOp = op; }

    base::Buffer& data() { return mData; }
    void setData(const base::Buffer& data) { mData = data; }

    virtual void dump(std::ostream& o) { o << "Op: " << mOp; }

protected:
    virtual void Serialize(base::Archive& ar) { ar << mOp << mData; }
    virtual void Deserialize(base::Archive& ar) { ar >> mOp >> mData; }

    OpCode mOp;
    base::Buffer mData;

private:
    ServerInfoCommand(CommandType type, CommandAction action) : Command(type, action) {}

};


class ServerActionSetPlayerCount : public base::Serializer {
public:
    int playercount;

protected:
    virtual void Serialize(base::Archive& ar) {
        ar << playercount;
    }
    virtual void Deserialize(base::Archive& ar) {
        ar >> playercount;
    }
};

class ServerActionAttachPlayers : public base::Serializer {
public:
    std::vector<std::string> players;
protected:
    virtual void Serialize(base::Archive& ar) {
        int count = players.size();
        ar << count;
        for (int i = 0; i < count; i++) {
            ar << players[i];
        }
    }
    virtual void Deserialize(base::Archive& ar) {
        int count = 0;
        ar >> count;
        players.reserve(count);
        for (int i = 0; i < count; i++) {
            ar >> players[i];
        }
    }
};

class ServerActionScore : public base::Serializer {
public:
    std::map<std::string, int> scores;
protected:
    virtual void Serialize(base::Archive& ar) {
        int count = scores.size();
        ar << count;
        for (auto it = scores.begin(); it != scores.end(); it++) {
            ar << it->first << it->second;
        }
    }
    virtual void Deserialize(base::Archive& ar) {
        int count = 0;
        ar >> count;
        for (int i = 0; i < count; i++) {
            std::string name;
            int score;
            ar >> name >> score;
            scores[name] = score;
        }
    }
};

} // namespace ib


