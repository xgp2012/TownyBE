#include "NationForms.h"
#include "data/DataStore.h"
#include "config/Config.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/service/Bedrock.h"
#include <sstream>

using namespace Towny::data;

void Towny::forms::openNationMenu(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());

    auto form = std::make_shared<ll::form::SimpleForm>(
        "Towny - 国家管理",
        "欢迎来到国家管理系统\n\n"
        "只有城镇镇长可以创建国家。\n"
        "国家由多个城镇组成，是最高级别的势力。"
    );

    if (!record || record->nationUuid.empty()) {
        form->setContent("你尚未加入任何国家。\n");
        form->appendButton("\u25c6 创建国家\n只有城镇镇长可以创建国家");
    } else {
        auto nation = store.getNation(record->nationUuid);
        if (nation) {
            std::ostringstream desc;
            desc << "当前所属国家: " << nation->name << "\n";
            desc << "城镇数: " << nation->getTownCount() << "\n";
            desc << "首都: " << store.getTownName(nation->capitalTownUuid);
            form->setContent(desc.str());
        }
        form->appendButton("\u25c6 管理国家\n管理城镇和首都");
    }

    form->appendButton("\u25c6 返回");

    form->sendTo(player, [](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        auto& store = DataStore::getInstance();
        auto* record = store.getPlayerRecord(p.getXuid());

        if (index == 0) {
            if (!record || record->nationUuid.empty()) {
                // 检查是否是城镇镇长
                if (record && !record->townUuid.empty()) {
                    auto town = store.getTown(record->townUuid);
                    if (town && town->leaderUuid == p.getXuid()) {
                        openCreateNationForm(p);
                    } else {
                        p.sendMessage("\u00a7c只有城镇镇长才能创建国家！");
                        openNationMenu(p);
                    }
                } else {
                    p.sendMessage("\u00a7c你尚未加入任何城镇，无法创建国家！");
                    openTownMenu(p);
                }
            }
        } else if (index == 1 && record && !record->nationUuid.empty()) {
            openManageNationForm(p);
        } else if (index == 2) {
            // 返回城镇主菜单
            openTownMenu(p);
        }
    });
}

void Towny::forms::openCreateNationForm(mc::Player& player) {
    auto form = std::make_shared<ll::form::SimpleForm>(
        "创建国家",
        "请输入国家名称：\n\n"
        "国家是最高级别的势力组织。\n"
        "国家名称在全服必须是唯一的。"
    );

    form->appendButton("\u25c6 创建\n使用输入的名称创建国家");
    form->appendButton("\u25c6 取消");

    form->sendTo(player, [](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;
        if (index != 0) {
            openNationMenu(p);
            return;
        }

        auto customForm = std::make_shared<ll::form::CustomForm>("创建国家");
        customForm->addLabel("请输入国家名称：");
        customForm->addInput("国家名称", "例如：大秦帝国");
        customForm->sendTo(p, [](mc::Player& p, const std::vector<std::string>& values, ll::form::FormCancelReason reason) {
            if (reason) return;
            if (values.empty()) return;

            auto& store = DataStore::getInstance();
            auto nationName = values[0];
            if (nationName.empty()) return;

            // 检查重名
            for (auto& [uuid, nation] : store.getAllNations()) {
                if (nation->name == nationName) {
                    p.sendMessage("\u00a7c该国家名称已存在！");
                    openCreateNationForm(p);
                    return;
                }
            }

            auto* record = store.getPlayerRecord(p.getXuid());
            if (!record || record->townUuid.empty()) {
                p.sendMessage("\u00a7c你必须先加入一个城镇！");
                return;
            }

            auto town = store.getTown(record->townUuid);
            if (!town) {
                p.sendMessage("\u00a7c你所属的城镇不存在！");
                return;
            }

            auto nationUuid = store.createNation(nationName, p.getXuid(), p.getName());
            if (nationUuid.empty()) {
                p.sendMessage("\u00a7c国家创建失败！");
                return;
            }

            auto nation = store.getNation(nationUuid);
            if (nation) {
                nation->addTown(town->uuid, town->name);
                store.saveNations();

                // 将该城镇所有成员关联到国家
                for (auto& member : town->members) {
                    store.setPlayerNation(member.uuid, nationUuid, nationName);
                }
            }

            p.sendMessage("\u00a7a国家 [\u00a7e" + nationName + "\u00a7a] 创建成功！");

            if (auto server = ll::service::getServerInstance()) {
                server->broadcastMessage("\u00a7e[\u00a76国家公告\u00a7e] \u00a7f" + p.getName() + " 创建了国家 [\u00a7e" + nationName + "\u00a7f]");
            }

            openNationMenu(p);
        });
    });
}

void Towny::forms::openManageNationForm(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());
    if (!record || record->nationUuid.empty()) {
        openNationMenu(player);
        return;
    }

    auto nation = store.getNation(record->nationUuid);
    if (!nation) {
        openNationMenu(player);
        return;
    }

    auto form = std::make_shared<ll::form::SimpleForm>(
        "\u00a7e管理国家: " + nation->name,
        "\u00a77城镇数: " + std::to_string(nation->getTownCount()) + "\n"
        "\u00a77首都: " + store.getTownName(nation->capitalTownUuid) + "\n\n"
        "\u00a77--- 所属城镇 ---\n"
    );

    // 构建城镇列表
    std::ostringstream townList;
    for (auto& t : nation->towns) {
        std::string capitalMark = (t.uuid == nation->capitalTownUuid) ? " \u00a7e\u2605" : "";
        townList << "\u00a77  " << t.name << capitalMark << "\n";
    }
    form->setContent(form->content() + townList.str());

    form->appendButton("\u25c6 管理城镇\n添加/移除城镇、设置首都");
    form->appendButton("\u25c6 解散国家\n解散当前国家");
    form->appendButton("\u25c6 返回");

    form->sendTo(player, [nation](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        auto& store = DataStore::getInstance();

        if (index == 0) {
            openNationTownManagement(p);
        } else if (index == 1) {
            // 解散国家确认
            auto confirmForm = std::make_shared<ll::form::ModalForm>(
                "解散国家",
                "你确定要解散国家 [" + nation->name + "] 吗？\n此操作不可恢复！",
                "\u00a7c确定解散",
                "\u00a77取消"
            );
            confirmForm->sendTo(p, [nation](mc::Player& p, ll::form::ModalFormSelectedButton result, ll::form::FormCancelReason reason) {
                if (reason) {
                    openManageNationForm(p);
                    return;
                }
                if (result == ll::form::ModalFormSelectedButton::Upper) {
                    auto& store = DataStore::getInstance();
                    auto* record = store.getPlayerRecord(p.getXuid());
                    if (record) {
                        store.deleteNation(record->nationUuid);
                    }
                    p.sendMessage("\u00a7c国家 [" + nation->name + "] 已解散！");

                    if (auto server = ll::service::getServerInstance()) {
                        server->broadcastMessage("\u00a7e[\u00a76国家公告\u00a7e] \u00a7f国家 [\u00a7c" + nation->name + "\u00a7f] 已解散");
                    }
                }
                openNationMenu(p);
            });
        } else {
            openNationMenu(player);
        }
    });
}

void Towny::forms::openNationTownManagement(mc::Player& player) {
    auto& store = DataStore::getInstance();
    auto* record = store.getPlayerRecord(player.getXuid());
    if (!record || record->nationUuid.empty()) {
        openNationMenu(player);
        return;
    }

    auto nation = store.getNation(record->nationUuid);
    if (!nation) {
        openNationMenu(player);
        return;
    }

    auto form = std::make_shared<ll::form::SimpleForm>(
        "管理国家城镇: " + nation->name,
        "选择要执行的操作："
    );

    form->appendButton("\u25c6 添加城镇\n添加已有城镇到国家");
    form->appendButton("\u25c6 设置首都\n选择首都城镇");
    form->appendButton("\u25c6 移除城镇\n从国家中移除城镇");
    form->appendButton("\u25c6 返回");

    form->sendTo(player, [nation](mc::Player& p, int index, ll::form::FormCancelReason reason) {
        if (reason) return;

        auto& store = DataStore::getInstance();

        if (index == 0) {
            // 添加城镇
            auto addForm = std::make_shared<ll::form::SimpleForm>(
                "添加城镇",
                "选择要添加到国家的城镇："
            );

            for (auto& [uuid, town] : store.getAllTowns()) {
                // 只添加尚未属于该国家的城镇
                if (!nation->hasTown(uuid)) {
                    addForm->appendButton("\u25c6 " + town->name);
                }
            }
            addForm->appendButton("\u25c6 返回");

            addForm->sendTo(p, [nation](mc::Player& p, int index, ll::form::FormCancelReason reason) {
                if (reason) return;
                openNationTownManagement(p);
            });
        } else if (index == 1) {
            // 设置首都
            auto capitalForm = std::make_shared<ll::form::SimpleForm>(
                "设置首都",
                "选择首都城镇："
            );

            for (auto& t : nation->towns) {
                std::string current = (t.uuid == nation->capitalTownUuid) ? " \u00a7e(首都)" : "";
                capitalForm->appendButton("\u25c6 " + t.name + current);
            }
            capitalForm->appendButton("\u25c6 返回");

            capitalForm->sendTo(p, [nation](mc::Player& p, int index, ll::form::FormCancelReason reason) {
                if (reason) return;
                openNationTownManagement(p);
            });
        } else if (index == 2) {
            // 移除城镇
            auto removeForm = std::make_shared<ll::form::SimpleForm>(
                "移除城镇",
                "选择要从国家移除的城镇："
            );

            for (auto& t : nation->towns) {
                removeForm->appendButton("\u25c6 " + t.name);
            }
            removeForm->appendButton("\u25c6 返回");

            removeForm->sendTo(p, [nation](mc::Player& p, int index, ll::form::FormCancelReason reason) {
                if (reason) return;
                openNationTownManagement(p);
            });
        } else {
            openManageNationForm(player);
        }
    });
}
