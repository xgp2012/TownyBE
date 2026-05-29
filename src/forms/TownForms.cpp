#include "TownForms.h"
#include "data/DataStore.h"
#include "config/Config.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/service/Bedrock.h"
#include <sstream>

using namespace Towny::data;
using namespace Towny::forms;

// 获取玩家当前所属城镇 UUID
std::string TownForms::getPlayerTownUuid(const mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());
    if (record) return record->townUuid;
    return "";
}

void TownForms::openTownMenu(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());

    auto form = std::make_shared<ll::form::SimpleForm>(
        "Towny - 城镇管理",
        "欢迎来到城镇管理系统\n\n"
        "你可以创建城镇、管理城镇、加入/离开城镇。"
    );

    if (!record || record->townUuid.empty()) {
        form->appendButton("\u25c6 创建城镇\n点击创建你的第一个城镇");
        form->appendButton("\u25c6 加入城镇\n加入已有的城镇");
    } else {
        auto town = store.getTown(record->townUuid);
        if (town) {
            std::ostringstream desc;
            desc << "当前所属城镇: " << town->name << "\n";
            desc << "成员数: " << town->getMemberCount() << "\n\n";
            desc << "你是";
            if (town->leaderUuid == player.getXuid()) {
                desc << "镇长\n";
            } else {
                desc << "成员\n";
            }
            form->setContent(desc.str());
        }
        form->appendButton("\u25c6 管理城镇\n管理成员和设置");
        form->appendButton("\u25c6 离开城镇\n离开当前城镇");
    }

    form->appendButton("\u25c6 国家管理\n查看国家相关信息");

    form->sendTo(player, [this](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        auto& store = DataStore::getInstance();
        auto* record = store.getPlayerRecord(p.getXuid());

        if (index == 0) {
            if (!record || record->townUuid.empty()) {
                openCreateTownForm(p);
            } else {
                openManageTownForm(p);
            }
        } else if (index == 1) {
            if (!record || record->townUuid.empty()) {
                openJoinTownForm(p);
            } else {
                openManageTownForm(p);
            }
        } else if (index == 2) {
            if (record && !record->townUuid.empty()) {
                // 离开城镇
                auto town = store.getTown(record->townUuid);
                if (town) {
                    if (town->leaderUuid == p.getXuid() && town->getMemberCount() > 1) {
                        p.sendMessage("\u00a7c你必须先转移镇长权限才能离开城镇！");
                        openManageTownForm(p);
                        return;
                    }
                    town->removeMember(p.getXuid());
                    store.setPlayerTown(p.getXuid(), "", "");
                    // 如果玩家所属国家，清除国家关系
                    for (auto& [nUuid, nation] : store.getAllNations()) {
                        nation->removeTown(record->townUuid);
                    }
                    store.saveNations();
                    p.sendMessage("\u00a7a你已离开城镇！");
                }
            }
            openTownMenu(p);
        } else if (index == 3) {
            // 跳转到国家管理
            forms::openNationMenu(p);
        }
    });
}

void TownForms::openCreateTownForm(mc::Player& player) {
    auto form = std::make_shared<ll::form::SimpleForm>(
        "创建城镇",
        "请输入城镇名称：\n\n"
        "城镇名称将作为你的国家基础。\n"
        "城镇名称在全服必须是唯一的。"
    );

    form->appendButton("\u25c6 创建\n使用输入的名称创建城镇");
    form->appendButton("\u25c6 取消");

    // 保存玩家回调信息到玩家自定义数据
    auto* instance = ll::service::getMinecraft();
    if (instance) {
        form->sendTo(player, [](mc::Player& p, int index, ll::form::FormCancelReason reason) {
            if (reason) return;
            if (index != 0) {
                openTownMenu(p);
                return;
            }

            // 获取表单输入（通过 CustomForm 实现）
            // 简化：这里打开一个 CustomForm 用于输入
            auto customForm = std::make_shared<ll::form::CustomForm>("创建城镇");
            customForm->addLabel("请输入城镇名称：");
            customForm->addInput("城镇名称", "例如：龙之谷");
            customForm->sendTo(p, [](mc::Player& p, const std::vector<std::string>& values, ll::form::FormCancelReason reason) {
                if (reason) return;
                if (values.empty()) return;

                auto& store = DataStore::getInstance();
                auto townName = values[0];
                if (townName.empty()) return;

                // 检查重名
                if (store.getTownByName(townName)) {
                    p.sendMessage("\u00a7c该城镇名称已存在！");
                    openCreateTownForm(p);
                    return;
                }

                auto townUuid = store.createTown(townName, p.getXuid(), p.getName());
                if (townUuid.empty()) {
                    p.sendMessage("\u00a7c城镇创建失败！");
                    return;
                }

                p.sendMessage("\u00a7a城镇 [\u00a7e" + townName + "\u00a7a] 创建成功！");

                // 通知所有在线玩家
                if (auto server = ll::service::getServerInstance()) {
                    server->broadcastMessage("\u00a7e[\u00a76城镇公告\u00a7e] \u00a7f" + p.getName() + " 创建了城镇 [\u00a7e" + townName + "\u00a7f]");
                }
            });
        });
    }
}

void TownForms::openManageTownForm(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());
    if (!record || record->townUuid.empty()) {
        openTownMenu(player);
        return;
    }

    auto town = store.getTown(record->townUuid);
    if (!town) {
        openTownMenu(player);
        return;
    }

    bool isLeader = (town->leaderUuid == player.getXuid());

    auto form = std::make_shared<ll::form::SimpleForm>(
        "\u00a7e管理城镇: " + town->name,
        "\u00a77成员数: " + std::to_string(town->getMemberCount()) + "\n"
    );

    // 构建成员列表描述
    std::ostringstream memberList;
    memberList << "\u00a77--- 成员列表 ---\n";
    for (auto& m : town->members) {
        if (m.uuid == town->leaderUuid) {
            memberList << "\u00a7e\u2605 " << m.name << " (镇长)\n";
        } else {
            memberList << "\u00a77  " << m.name << "\n";
        }
    }
    form->setContent(memberList.str());

    form->appendButton("\u25c6 邀请成员\n邀请玩家加入城镇");
    if (isLeader) {
        form->appendButton("\u25c6 设置镇长\n转移镇长权限");
        form->appendButton("\u25c6 移除成员\n移除城镇成员");
        form->appendButton("\u25c6 转移城镇\n将整个城镇转移");
        form->appendButton("\u25c6 解散城镇\n解散当前城镇");
    }
    form->appendButton("\u25c6 返回");

    form->sendTo(player, [isLeader](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        if (index == 0) {
            // 邀请成员 - 简化为命令行
            p.sendMessage("\u00a77请使用命令: \u00a7e/town invite <玩家名>");
            openManageTownForm(p);
        } else if (index == 1 && isLeader) {
            // 设置镇长
            auto town = store.getTown(record->townUuid);
            if (town && town->members.size() >= 2) {
                p.sendMessage("\u00a77请使用命令: \u00a7e/town setleader <玩家名>");
                openManageTownForm(p);
            }
        } else if (index == 2 && isLeader) {
            openMemberManagementForm(p);
        } else if (index == 3 && isLeader) {
            p.sendMessage("\u00a77请使用命令: \u00a7e/town transfer <玩家名>");
            openManageTownForm(p);
        } else if (index == 4 && isLeader) {
            // 解散城镇确认
            auto confirmForm = std::make_shared<ll::form::ModalForm>(
                "解散城镇",
                "你确定要解散城镇 [" + town->name + "] 吗？\n此操作不可恢复！",
                "\u00a7c确定解散",
                "\u00a77取消"
            );
            confirmForm->sendTo(p, [&](mc::Player& p, ll::form::ModalFormSelectedButton result, ll::form::FormCancelReason reason) {
                if (reason) {
                    openManageTownForm(p);
                    return;
                }
                if (result == ll::form::ModalFormSelectedButton::Upper) {
                    store.deleteTown(town->uuid);
                    p.sendMessage("\u00a7c城镇 [" + town->name + "] 已解散！");

                    if (auto server = ll::service::getServerInstance()) {
                        server->broadcastMessage("\u00a7e[\u00a76城镇公告\u00a7e] \u00a7f城镇 [\u00a7c" + town->name + "\u00a7f] 已解散");
                    }
                }
                openTownMenu(p);
            });
        } else {
            openTownMenu(player);
        }
    });
}

void TownForms::openMemberManagementForm(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());
    if (!record || record->townUuid.empty()) {
        openTownMenu(player);
        return;
    }

    auto town = store.getTown(record->townUuid);
    if (!town || town->leaderUuid != player.getXuid()) {
        openTownMenu(player);
        return;
    }

    auto form = std::make_shared<ll::form::SimpleForm>(
        "管理成员: " + town->name,
        "选择要执行的操作："
    );

    form->appendButton("\u25c6 邀请新成员\n邀请玩家加入");
    form->appendButton("\u25c6 移除成员\n从城镇中移除");
    form->appendButton("\u25c6 设置镇长\n转移镇长权限");
    form->appendButton("\u25c6 返回");

    form->sendTo(player, [](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        if (index == 0) {
            // 邀请成员
            p.sendMessage("\u00a77请使用命令: \u00a7e/town invite <玩家名>");
            openMemberManagementForm(p);
        } else if (index == 1) {
            // 移除成员 - 显示成员列表供选择
            auto& store = DataStore::getInstance();
            auto* rec = store.getPlayerRecord(p.getXuid());
            if (!rec || rec->townUuid.empty()) return;
            auto t = store.getTown(rec->townUuid);
            if (!t) return;

            auto removeForm = std::make_shared<ll::form::SimpleForm>(
                "移除成员",
                "选择要移除的成员："
            );

            for (auto& m : t->members) {
                if (m.uuid == t->leaderUuid) continue; // 不能移除镇长
                std::string label = "\u25c6 " + m.name;
                removeForm->appendButton(label);
            }
            removeForm->appendButton("\u25c6 返回");

            // 记录移除操作
            removeForm->sendTo(p, [t](mc::Player& p, int index, ll::form::FormCancelReason reason) {
                if (reason) return;
                if (static_cast<size_t>(index) >= t->members.size()) {
                    openMemberManagementForm(p);
                    return;
                }
                auto& store = DataStore::getInstance();
                auto* rec = store.getPlayerRecord(p.getXuid());
                if (!rec) return;
                auto town = store.getTown(rec->townUuid);
                if (!town) return;

                // 过滤非镇长成员
                size_t count = 0;
                for (auto& m : town->members) {
                    if (m.uuid == town->leaderUuid) continue;
                    if (count == index) {
                        if (town->removeMember(m.uuid)) {
                            store.setPlayerTown(m.uuid, "", "");
                            p.sendMessage("\u00a7a已将 " + m.name + " 从城镇中移除");
                        }
                        break;
                    }
                    count++;
                }
                openMemberManagementForm(p);
            });
        } else if (index == 2) {
            // 设置镇长
            p.sendMessage("\u00a77请使用命令: \u00a7e/town setleader <玩家名>");
            openMemberManagementForm(p);
        } else {
            openManageTownForm(player);
        }
    });
}

void TownForms::openJoinTownForm(mc::Player& player) {
    auto& store = DataStore::getInstance();

    auto form = std::make_shared<ll::form::SimpleForm>(
        "加入城镇",
        "当前可用城镇：\n"
    );

    bool hasTowns = false;
    for (auto& [uuid, town] : store.getAllTowns()) {
        form->appendButton("\u25c6 " + town->name + " (\u00a77成员: " + std::to_string(town->getMemberCount()) + "\u00a77)");
        hasTowns = true;
    }

    if (!hasTowns) {
        form->setContent("当前没有可用的城镇\n\n" + form->content());
    }

    form->appendButton("\u25c6 返回");

    form->sendTo(player, [hasTowns](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        if (index == static_cast<int>(store.getAllTowns().size())) {
            openTownMenu(p);
            return;
        }

        if (!hasTowns || static_cast<size_t>(index) >= store.getAllTowns().size()) return;

        size_t count = 0;
        for (auto& [uuid, town] : store.getAllTowns()) {
            if (count == index) {
                auto& store = DataStore::getInstance();
                // 将玩家添加到城镇
                town->addMember(p.getXuid(), p.getName());
                store.setPlayerTown(p.getXuid(), uuid, town->name);
                p.sendMessage("\u00a7a你已加入城镇 [\u00a7e" + town->name + "\u00a7a]！");
                break;
            }
            count++;
        }
        openJoinTownForm(p);
    });
}
