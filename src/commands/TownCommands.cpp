#include "TownCommands.h"
#include "data/DataStore.h"
#include "forms/TownForms.h"
#include "forms/NationForms.h"
#include "chat/ChatPrefix.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/service/Bedrock.h"
#include <sstream>

using namespace Towny::commands;
using namespace Towny::data;

void commands::registerTownCommands() {
    auto& registrar = ll::command::CommandRegistrar::getInstance(false);

    // /town (打开 GUI)
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload()
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result) {
            auto* player = origin.getPlayer();
            if (!player) return;
            forms::openTownMenu(*player);
        });

    // /town create <name>
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload<std::string>()
        .text("create")
        .required("name")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result, std::string name) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (record && !record->townUuid.empty()) {
                player->sendMessage("\u00a7c你已所属城镇 [" + record->townName + "]，无法创建新城镇！");
                return;
            }

            if (store.getTownByName(name)) {
                player->sendMessage("\u00a7c城镇名称 [" + name + "] 已被使用！");
                return;
            }

            auto townUuid = store.createTown(name, player->getXuid(), player->getName());
            if (townUuid.empty()) {
                player->sendMessage("\u00a7c城镇创建失败！");
                return;
            }

            player->sendMessage("\u00a7a城镇 [\u00a7e" + name + "\u00a7a] 创建成功！");

            if (auto server = ll::service::getServerInstance()) {
                server->broadcastMessage("\u00a7e[\u00a76城镇公告\u00a7e] \u00a7f" + player->getName() + " 创建了城镇 [\u00a7e" + name + "\u00a7f]");
            }
        });

    // /town leave
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload()
        .text("leave")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (!record || record->townUuid.empty()) {
                player->sendMessage("\u00a7c你尚未加入任何城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town) {
                player->sendMessage("\u00a7c你所属的城镇不存在！");
                return;
            }

            if (town->leaderUuid == player->getXuid() && town->getMemberCount() > 1) {
                player->sendMessage("\u00a7c你必须先转移镇长权限才能离开城镇！");
                return;
            }

            town->removeMember(player->getXuid());
            store.setPlayerTown(player->getXuid(), "", "");

            for (auto& [nUuid, nation] : store.getAllNations()) {
                nation->removeTown(record->townUuid);
            }
            store.saveNations();

            player->sendMessage("\u00a7a你已离开城镇 [" + record->townName + "]！");
        });

    // /town invite <player>
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload<std::string>()
        .text("invite")
        .required("player")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result, std::string targetName) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (!record || record->townUuid.empty()) {
                player->sendMessage("\u00a7c你尚未加入任何城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town || town->leaderUuid != player->getXuid()) {
                player->sendMessage("\u00a7c只有镇长才能邀请成员！");
                return;
            }

            // 查找目标玩家
            if (auto server = ll::service::getServerInstance()) {
                auto targetPlayer = server->findPlayer(targetName);
                if (!targetPlayer) {
                    player->sendMessage("\u00a7c找不到玩家 [" + targetName + "]！");
                    return;
                }

                auto* targetRecord = store.getPlayerRecord(targetPlayer->getXuid());
                if (targetRecord && !targetRecord->townUuid.empty()) {
                    player->sendMessage("\u00a7c玩家 [" + targetName + "] 已所属城镇！");
                    return;
                }

                town->addMember(targetPlayer->getXuid(), targetName);
                store.setPlayerTown(targetPlayer->getXuid(), town->uuid, town->name);

                targetPlayer->sendMessage("\u00a7a你已被邀请加入城镇 [\u00a7e" + town->name + "\u00a7a]！");
                player->sendMessage("\u00a7a已将 \u00a7e" + targetName + "\u00a7a 邀请加入城镇！");
            } else {
                player->sendMessage("\u00a7c服务器不可用！");
            }
        });

    // /town setleader <player>
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload<std::string>()
        .text("setleader")
        .required("player")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result, std::string targetName) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (!record || record->townUuid.empty()) {
                player->sendMessage("\u00a7c你尚未加入任何城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town || town->leaderUuid != player->getXuid()) {
                player->sendMessage("\u00a7c只有镇长才能设置镇长！");
                return;
            }

            // 查找目标玩家
            if (auto server = ll::service::getServerInstance()) {
                auto targetPlayer = server->findPlayer(targetName);
                if (!targetPlayer) {
                    player->sendMessage("\u00a7c找不到玩家 [" + targetName + "]！");
                    return;
                }

                if (!town->hasMember(targetPlayer->getXuid())) {
                    player->sendMessage("\u00a7c玩家 [" + targetName + "] 不是城镇成员！");
                    return;
                }

                town->setLeader(targetPlayer->getXuid());
                player->sendMessage("\u00a7a已将镇长权限转移给 \u00a7e" + targetName + "\u00a7a！");
            }
        });

    // /town disband
    registrar.getOrCreateCommand("town", "城镇管理系统", ll::command::CommandPermissionLevel::Any)
        .overload()
        .text("disband")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (!record || record->townUuid.empty()) {
                player->sendMessage("\u00a7c你尚未加入任何城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town || town->leaderUuid != player->getXuid()) {
                player->sendMessage("\u00a7c只有镇长才能解散城镇！");
                return;
            }

            store.deleteTown(town->uuid);
            player->sendMessage("\u00a7c城镇 [\u00a7c" + town->name + "\u00a7c] 已解散！");

            if (auto server = ll::service::getServerInstance()) {
                server->broadcastMessage("\u00a7e[\u00a76城镇公告\u00a7e] \u00a7f城镇 [\u00a7c" + town->name + "\u00a7f] 已解散");
            }
        });
}

void commands::registerNationCommands() {
    auto& registrar = ll::command::CommandRegistrar::getInstance(false);

    // /nation (打开 GUI)
    registrar.getOrCreateCommand("nation", "国家管理系统", ll::command::CommandPermissionLevel::Any)
        .overload()
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result) {
            auto* player = origin.getPlayer();
            if (!player) return;
            forms::openNationMenu(*player);
        });

    // /nation create <name>
    registrar.getOrCreateCommand("nation", "国家管理系统", ll::command::CommandPermissionLevel::Any)
        .overload<std::string>()
        .text("create")
        .required("name")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result, std::string name) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (record && !record->nationUuid.empty()) {
                player->sendMessage("\u00a7c你已所属国家 [" + record->nationName + "]，无法创建新国家！");
                return;
            }

            if (!record || record->townUuid.empty()) {
                player->sendMessage("\u00a7c你必须先加入一个城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town) {
                player->sendMessage("\u00a7c你所属的城镇不存在！");
                return;
            }

            if (town->leaderUuid != player->getXuid()) {
                player->sendMessage("\u00a7c只有城镇镇长才能创建国家！");
                return;
            }

            for (auto& [nUuid, nation] : store.getAllNations()) {
                if (nation->name == name) {
                    player->sendMessage("\u00a7c国家名称 [" + name + "] 已被使用！");
                    return;
                }
            }

            auto nationUuid = store.createNation(name, player->getXuid(), player->getName());
            if (nationUuid.empty()) {
                player->sendMessage("\u00a7c国家创建失败！");
                return;
            }

            auto nation = store.getNation(nationUuid);
            if (nation) {
                nation->addTown(town->uuid, town->name);
                store.saveNations();

                for (auto& member : town->members) {
                    store.setPlayerNation(member.uuid, nationUuid, name);
                }
            }

            player->sendMessage("\u00a7a国家 [\u00a7e" + name + "\u00a7a] 创建成功！");

            if (auto server = ll::service::getServerInstance()) {
                server->broadcastMessage("\u00a7e[\u00a76国家公告\u00a7e] \u00a7f" + player->getName() + " 创建了国家 [\u00a7e" + name + "\u00a7f]");
            }
        });

    // /nation disband
    registrar.getOrCreateCommand("nation", "国家管理系统", ll::command::CommandPermissionLevel::Any)
        .overload()
        .text("disband")
        .execute([](ll::command::CommandOrigin& origin, ll::command::CommandReturnValue& result) {
            auto* player = origin.getPlayer();
            if (!player) return;

            auto& store = DataStore::getInstance();
            auto* record = store.getPlayerRecord(player->getXuid());

            if (!record || record->nationUuid.empty()) {
                player->sendMessage("\u00a7c你尚未加入任何国家！");
                return;
            }

            auto nation = store.getNation(record->nationUuid);
            if (!nation) {
                player->sendMessage("\u00a7c你所属的国家不存在！");
                return;
            }

            store.deleteNation(record->nationUuid);
            player->sendMessage("\u00a7c国家 [\u00a7c" + nation->name + "\u00a7c] 已解散！");

            if (auto server = ll::service::getServerInstance()) {
                server->broadcastMessage("\u00a7e[\u00a76国家公告\u00a7e] \u00a7f国家 [\u00a7c" + nation->name + "\u00a7f] 已解散");
            }
        });
}
