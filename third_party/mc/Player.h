#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>

namespace mc {
    class Player {
    public:
        std::string getXuid() const { return ""; }
        std::string getName() const { return ""; }
        int getX() const { return 0; }
        int getY() const { return 0; }
        int getZ() const { return 0; }
        void sendMessage(const std::string& msg) const {}
        std::map<std::string, std::string>& getCustomData() { return customData_; }
    private:
        std::map<std::string, std::string> customData_;
    };
}
