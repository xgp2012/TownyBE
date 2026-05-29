#pragma once
#include <string>
#include <vector>
#include <memory>

namespace Towny::data {

struct TownMember {
    std::string uuid;
    std::string name;
};

class Town {
public:
    std::string uuid;
    std::string name;
    std::string founderUuid;
    std::string founderName;
    std::string leaderUuid;
    std::vector<TownMember> members;

    Town() = default;
    Town(std::string u, std::string n, std::string fu, std::string fn)
        : uuid(std::move(u)), name(std::move(n)),
          founderUuid(std::move(fu)), founderName(std::move(fn)),
          leaderUuid(std::move(fu)) {}

    bool hasMember(const std::string& uuid) const;
    bool addMember(const std::string& uuid, const std::string& name);
    bool removeMember(const std::string& uuid);
    void setLeader(const std::string& uuid);
    int getMemberCount() const { return static_cast<int>(members.size()); }
};

} // namespace Towny::data
