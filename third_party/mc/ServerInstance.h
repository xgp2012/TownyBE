#pragma once
#include <string>
#include <cstdint>
#include "mc/Player.h"

namespace mc {
    class ServerInstance {
    public:
        virtual ~ServerInstance() = default;
        virtual mc::Player* findPlayer(const std::string& name) = 0;
        virtual void broadcastMessage(const std::string& msg) = 0;
    };
}
