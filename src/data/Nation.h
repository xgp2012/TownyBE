#pragma once
#include <string>
#include <vector>
#include <memory>

namespace Towny::data {

struct NationTown {
    std::string uuid;
    std::string name;
};

class Nation {
public:
    std::string uuid;
    std::string name;
    std::string founderUuid;
    std::string founderName;
    std::string capitalTownUuid;
    std::vector<NationTown> towns;

    Nation() = default;
    Nation(std::string u, std::string n, std::string fu, std::string fn)
        : uuid(std::move(u)), name(std::move(n)),
          founderUuid(std::move(fu)), founderName(std::move(fn)) {}

    bool hasTown(const std::string& townUuid) const;
    bool addTown(const std::string& uuid, const std::string& name);
    bool removeTown(const std::string& uuid);
    void setCapital(const std::string& townUuid);
    int getTownCount() const { return static_cast<int>(towns.size()); }
};

} // namespace Towny::data
