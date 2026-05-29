#pragma once
#include <string>

namespace Towny::chat {

// 初始化聊天前缀系统
void init();

// 处理聊天事件，添加前缀并修改消息
void onChat(const std::string& playerName, const std::string& message);

// 生成聊天前缀字符串
std::string generatePrefix(const std::string& playerName);

} // namespace Towny::chat
