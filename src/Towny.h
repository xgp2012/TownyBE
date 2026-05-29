#pragma once
#include <filesystem>
#include <string>

namespace Towny::mod {
class Logger;
}

namespace Towny {

class TownyMod {
public:
    TownyMod() = default;
    ~TownyMod() = default;

    bool load();
    bool enable();
    bool disable();
    bool unload();

    mod::Logger& getLogger();

private:
    std::filesystem::path dataPath_;
    bool initialized_ = false;
};

} // namespace Towny
