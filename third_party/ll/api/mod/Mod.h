#define LL_SHARED_EXPORT __declspec(dllexport)
#pragma once
#include <string>
#include <memory>

namespace ll {
namespace mod {

class Mod {
public:
    virtual ~Mod() = default;
    std::string loadResourceAsString(const std::string& name) { return ""; }
};

class NativeMod {
public:
    virtual ~NativeMod() = default;
    static NativeMod* current();
    std::filesystem::path getConfigDir() const { return configDir_; }
    std::filesystem::path getDataDir() const { return dataDir_; }
    template<typename F>
    void onEnable(F&& f) {}
    template<typename F>
    void onDisable(F&& f) {}
    template<typename F>
    void onUnload(F&& f) {}
    void getLogger() { return Logger(); }
private:
    std::filesystem::path configDir_;
    std::filesystem::path dataDir_;
};

class Logger {
public:
    template<typename... Args>
    void info(Args&&...) {}
    template<typename... Args>
    void warn(Args&&...) {}
    template<typename... Args>
    void error(Args&&...) {}
};

}
}
