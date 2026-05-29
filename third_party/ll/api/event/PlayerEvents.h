#pragma once
#include <string>
#include <functional>

namespace ll {
namespace event {

struct PlayerChatEvent {
    std::string& getMessage() { return msg; }
    void setCancelled(bool) {}
    mc::Player& getPlayer() { return player; }
    mc::Player& getSender() { return player; }
    mc::ServerInstance& getServer() { return *server; }
private:
    std::string msg;
    mc::Player player;
    mc::ServerInstance* server = nullptr;
};

struct PlayerJoinEvent {
    mc::Player& getPlayer() { return player; }
private:
    mc::Player player;
};

struct PlayerDisconnectEvent {
    mc::Player& getPlayer() { return player; }
private:
    mc::Player player;
};

}
}
