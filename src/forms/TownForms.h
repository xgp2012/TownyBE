#pragma once
#include <string>
#include <functional>

namespace ll::form {
class SimpleForm;
}
namespace mc {
class Player;
}
namespace Towny::forms {

// 获取玩家所属城镇 UUID（空字符串表示无城镇）
std::string getPlayerTownUuid(const mc::Player& player);

// 打开城镇主菜单 GUI
void openTownMenu(mc::Player& player);

// 打开创建城镇 GUI
void openCreateTownForm(mc::Player& player);

// 打开管理城镇 GUI
void openManageTownForm(mc::Player& player);

// 打开成员管理 GUI
void openMemberManagementForm(mc::Player& player);

// 打开加入城镇 GUI
void openJoinTownForm(mc::Player& player);

} // namespace Towny::forms
