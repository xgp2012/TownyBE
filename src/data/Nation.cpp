#include "Nation.h"

namespace Towny::data {

bool Nation::hasTown(const std::string& townUuid) const {
    for (auto& t : towns) {
        if (t.uuid == townUuid) return true;
    }
    return false;
}

bool Nation::addTown(const std::string& uuid, const std::string& name) {
    if (hasTown(uuid)) return false;
    towns.push_back({uuid, name});
    if (capitalTownUuid.empty()) {
        capitalTownUuid = uuid;
    }
    return true;
}

bool Nation::removeTown(const std::string& uuid) {
    for (auto it = towns.begin(); it != towns.end(); ++it) {
        if (it->uuid == uuid) {
            if (towns.size() <= 1) return false;
            if (capitalTownUuid == uuid) {
                capitalTownUuid = towns.empty() ? "" : towns.front().uuid;
            }
            towns.erase(it);
            return true;
        }
    }
    return false;
}

void Nation::setCapital(const std::string& townUuid) {
    if (hasTown(townUuid)) {
        capitalTownUuid = townUuid;
    }
}

} // namespace Towny::data
