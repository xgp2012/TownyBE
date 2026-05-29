#include "ChatPrefix.h"
#include "data/DataStore.h"
#include "config/Config.h"
#include "ll/api/event/PlayerChatEvent.h"
#include "ll/api/event/PlayerJoinEvent.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/mod/Mod.h"
#include <sstream>
#include <algorithm>
#include <fstream>

using namespace Towny::data;
using namespace Towny::chat;

static void onPlayerChat(PlayerChatEvent& ev) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(ev.getPlayer().getXuid());

    std::string prefix;
    std::string townName = config::defaultConfig.noTown;
    std::string nationName = config::defaultConfig.noNation;

    if (record) {
        if (!record->townUuid.empty()) {
            townName = record->townName.empty() ? store.getTownName(record->townUuid) : record->townName;
        }
        if (!record->nationUuid.empty()) {
            nationName = record->nationName.empty() ? store.getNationName(record->nationUuid) : record->nationName;
        }
    }

    // 生成前缀
    std::string format = config::defaultConfig.chatPrefixFormat;
    // 替换 {town} 和 {nation} 占位符
    std::replace(format.begin(), format.end(), '{', '[');
    std::replace(format.begin(), format.end(), '}', ']');
    std::string townPart = "[" + townName + "]";
    std::string nationPart = "[" + nationName + "]";

    // 简单替换
    std::string prefixStr;
    if (nationName != config::defaultConfig.noNation) {
        prefixStr = "\u00a7e[" + nationName + "\u00a77]\u00a7e[" + townName + "\u00a77]\u00b7";
    } else {
        prefixStr = "\u00a7e[" + townName + "\u00a77]\u00b7";
    }

    ev.setCancelled(true);
    ev.getPlayer().sendMessage(prefixStr + ev.getMessage());
    ev.getSender().getServer().broadcastMessage(prefixStr + ev.getMessage());
}

static void onPlayerJoin(PlayerJoinEvent& ev) {
    auto& store = DataStore::getInstance();
    store.registerPlayer(ev.getPlayer().getXuid(), ev.getPlayer().getName());
}

void Towny::chat::init() {
    EventBus& bus = EventBus::getInstance();
    bus.emplaceListener<PlayerChatEvent>([](PlayerChatEvent& ev) {
        auto& store = DataStore::getInstance();
        auto* record = store.getPlayerRecord(ev.getPlayer().getXuid());

        std::string townName = config::defaultConfig.noTown;
        std::string nationName = config::defaultConfig.noNation;

        if (record) {
            if (!record->townUuid.empty()) {
                townName = record->townName.empty() ? store.getTownName(record->townUuid) : record->townName;
            }
            if (!record->nationUuid.empty()) {
                nationName = record->nationName.empty() ? store.getNationName(record->nationUuid) : record->nationName;
            }
        }

        // 构建前缀
        std::string prefixStr;
        if (nationName != config::defaultConfig.noNation) {
            prefixStr = "\u00a7e[" + nationName + "\u00a77]\u00a7e[" + townName + "\u00a77]\u00b7";
        } else {
            prefixStr = "\u00a7e[" + townName + "\u00a77]\u00b7";
        }

        ev.setCancelled(true);
        ev.getSender().getServer().broadcastMessage(prefixStr + ev.getMessage());
    });

    bus.emplaceListener<PlayerJoinEvent>([](PlayerJoinEvent& ev) {
        auto& store = DataStore::getInstance();
        store.registerPlayer(ev.getPlayer().getXuid(), ev.getPlayer().getName());
    });
}

void Towny::chat::onChat(const std::string& playerName, const std::string& message) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(playerName);

    std::string townName = config::defaultConfig.noTown;
    std::string nationName = config::defaultConfig.noNation;

    if (record) {
        if (!record->townUuid.empty()) {
            townName = record->townName.empty() ? store.getTownName(record->townUuid) : record->townName;
        }
        if (!record->nationUuid.empty()) {
            nationName = record->nationName.empty() ? store.getNationName(record->nationUuid) : record->nationName;
        }
    }

    std::string prefixStr;
    if (nationName != config::defaultConfig.noNation) {
        prefixStr = "\u00a7e[" + nationName + "\u00a77]\u00a7e[" + townName + "\u00a77]\u00b7";
    } else {
        prefixStr = "\u00a7e[" + townName + "\u00a77]\u00b7";
    }

    if (auto server = ll::service::getServerInstance()) {
        server->broadcastMessage(prefixStr + message);
    }
}

std::string Towny::chat::generatePrefix(const std::string& playerName) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(playerName);

    std::string townName = config::defaultConfig.noTown;
    std::string nationName = config::defaultConfig.noNation;

    if (record) {
        if (!record->townUuid.empty()) {
            townName = record->townName.empty() ? store.getTownName(record->townUuid) : record->townName;
        }
        if (!record->nationUuid.empty()) {
            nationName = record->nationName.empty() ? store.getNationName(record->nationUuid) : record->nationName;
        }
    }

    std::string prefixStr;
    if (nationName != config::defaultConfig.noNation) {
        prefixStr = "\u00a7e[" + nationName + "\u00a77]\u00a7e[" + townName + "\u00a77]\u00b7";
    } else {
        prefixStr = "\u00a7e[" + townName + "\u00a77]\u00b7";
    }

    return prefixStr;
}
