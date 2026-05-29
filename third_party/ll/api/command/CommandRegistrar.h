#pragma once
#include <string>
#include "ll/api/mod/Mod.h"

namespace ll {
namespace command {

enum class CommandPermissionLevel : int {
    Any,
    Unknown,
    Operator,
    Internal,
};

class CommandOrigin {
public:
    mc::Player* getPlayer() const { return nullptr; }
    std::string getName() const { return ""; }
};

class CommandReturnValue {};

class CommandRegistrar {
public:
    static CommandRegistrar& getInstance(bool create = true);
    CommandHandle& getOrCreateCommand(const std::string& name, const std::string& desc, CommandPermissionLevel perm);
};

}
}
