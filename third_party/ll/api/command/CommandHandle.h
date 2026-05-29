#pragma once
#include <functional>
#include <vector>
#include <string>
#include "ll/api/command/CommandRegistrar.h"

namespace ll {
namespace command {

struct CommandHandle {
    CommandHandle& overload();
    CommandHandle& text(const std::string& name);
    CommandHandle& required(const std::string& name);
    CommandHandle& text(const std::string& subcmd) { return *this; }
    CommandHandle& required(const std::string& param) { return *this; }
    template<typename T>
    CommandHandle& overload() { return *this; }
    template<typename... Args>
    CommandHandle& execute(Args&&...) { return *this; }
};

}
}
