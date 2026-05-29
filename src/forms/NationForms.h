#pragma once
#include <string>

namespace mc {
class Player;
}
namespace ll::form {
class SimpleForm;
}
namespace Towny::forms {

// 打开国家主菜单 GUI
void openNationMenu(mc::Player& player);

// 打开创建国家 GUI
void openCreateNationForm(mc::Player& player);

// 打开管理国家 GUI
void openManageNationForm(mc::Player& player);

// 打开城镇管理 GUI (国家层面)
void openNationTownManagement(mc::Player& player);

} // namespace Towny::forms
