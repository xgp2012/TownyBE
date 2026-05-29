#include "Town.h"

namespace Towny::data {

bool Town::hasMember(const std::string& uuid) const {
    for (auto& m : members) {
        if (m.uuid == uuid) return true;
    }
    return false;
}

bool Town::addMember(const std::string& uuid, const std::string& name) {
    if (hasMember(uuid)) return false;
    members.push_back({uuid, name});
    return true;
}

bool Town::removeMember(const std::string& uuid) {
    for (auto it = members.begin(); it != members.end(); ++it) {
        if (it->uuid == uuid) {
            // Don't allow removing the last member
            if (members.size() <= 1) return false;
            members.erase(it);
            return true;
        }
    }
    return false;
}

void Town::setLeader(const std::string& uuid) {
    if (hasMember(uuid)) {
        leaderUuid = uuid;
    }
}

} // namespace Towny::data
