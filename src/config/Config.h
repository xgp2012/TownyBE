#pragma once
#include <string>

namespace Towny::config {

struct Config {
    // 聊天前缀格式
    std::string chatPrefixFormat = "[{nation}]{town}\u00b7";
    // 无城镇/国家时的占位符
    std::string noTown = "无城镇";
    std::string noNation = "无国家";
    // 权限设置
    bool onlyTownLeaderCanCreateNation = true;
    // 消息颜色 (Minecraft 颜色代码)
    std::string prefixColor = "\u00a7e";  // 金色
    std::string separatorColor = "\u00a77";  // 灰色
};

} // namespace Towny::config
