#include "Towny.h"
#include "config/Config.h"
#include "data/DataStore.h"
#include "chat/ChatPrefix.h"
#include "commands/TownCommands.h"
#include "forms/TownForms.h"
#include "forms/NationForms.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/Config.h"

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using namespace Towny;

static config::Config gConfig;

static void loadConfig(ll::mod::Mod& mod) {
    auto configPath = mod.getConfigDir() / "config.json";
    auto content = mod.loadResourceAsString("config.json");
    if (!content.has_value() || content->empty()) {
        // 尝试从文件系统加载
        std::ifstream file(configPath);
        if (file.is_open()) {
            std::stringstream ss;
            ss << file.rdbuf();
            content = ss.str();
        }
    }

    if (content.has_value() && !content->empty()) {
        try {
            auto j = nlohmann::json::parse(*content);
            gConfig.chatPrefixFormat = j.value("chatPrefixFormat", "[{nation}]{town}\u00b7");
            gConfig.noTown = j.value("noTown", "无城镇");
            gConfig.noNation = j.value("noNation", "无国家");
            gConfig.onlyTownLeaderCanCreateNation = j.value("onlyTownLeaderCanCreateNation", true);
        } catch (...) {
            // 使用默认配置
        }
    } else {
        // 保存默认配置
        nlohmann::json j;
        j["chatPrefixFormat"] = gConfig.chatPrefixFormat;
        j["noTown"] = gConfig.noTown;
        j["noNation"] = gConfig.noNation;
        j["onlyTownLeaderCanCreateNation"] = gConfig.onlyTownLeaderCanCreateNation;
        std::ofstream file(configPath);
        file << j.dump(2);
    }
}

namespace Towny::config {
Config& getDefaultConfig() { return gConfig; }
const Config& defaultConfig = gConfig;
}

bool TownyMod::load() {
    return true;
}

bool TownyMod::enable() {
    if (initialized_) return true;

    // 获取 mod 实例
    auto mod = ll::mod::NativeMod::current();

    // 加载配置
    loadConfig(mod);

    // 初始化数据存储
    dataPath_ = mod.getDataDir();
    auto& store = data::DataStore::getInstance();
    store.init(dataPath_);

    // 初始化聊天前缀
    chat::init();

    // 注册命令
    commands::registerTownCommands();
    commands::registerNationCommands();

    initialized_ = true;
    return true;
}

bool TownyMod::disable() {
    // 保存所有数据
    auto& store = data::DataStore::getInstance();
    store.save();
    return true;
}

bool TownyMod::unload() {
    // 保存所有数据
    auto& store = data::DataStore::getInstance();
    store.save();
    initialized_ = false;
    return true;
}

extern "C" {
LL_SHARED_EXPORT bool ll_mod_load(ll::mod::NativeMod& self) {
    static TownyMod mod;
    mod.getLogger().info("Towny plugin is loading...");

    // 手动绑定生命周期
    self.onEnable({ &mod [](TownyMod& m) -> bool { return m.enable(); } });
    self.onDisable({ &mod [](TownyMod& m) -> bool { return m.disable(); } });
    self.onUnload({ &mod [](TownyMod& m) -> bool { return m.unload(); } });

    return true;
}
}
