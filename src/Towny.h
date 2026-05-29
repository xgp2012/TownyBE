#pragma once
#include <filesystem>

namespace Towny {

class TownyMod {
public:
    TownyMod() = default;
    ~TownyMod() = default;

    bool load();
    bool enable();
    bool disable();
    bool unload();

private:
    std::filesystem::path dataPath_;
    bool initialized_ = false;
};

} // namespace Towny
