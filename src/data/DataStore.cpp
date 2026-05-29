#include "DataStore.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace Towny::data {

// UUID 工具函数
static std::string generateUuid() {
    // 生成简单唯一 ID (实际项目中应使用 UUID4)
    static unsigned long long counter = 0;
    std::ostringstream oss;
    oss << "uuid-" << ++counter << "-" << std::hex << std::rand();
    return oss.str();
}

static std::string loadJsonFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static bool saveJsonFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return true;
}

DataStore& DataStore::getInstance() {
    static DataStore instance;
    return instance;
}

void DataStore::init(const std::filesystem::path& basePath) {
    basePath_ = basePath;
    loadTowns();
    loadNations();
    loadPlayers();
}

void DataStore::save() {
    std::lock_guard<std::mutex> lock(mutex_);
    saveTowns();
    saveNations();
    savePlayers();
}

void DataStore::loadTowns() {
    auto path = basePath_ / "towns.json";
    auto content = loadJsonFile(path);
    if (content.empty()) return;
    try {
        nlohmann::json j = nlohmann::json::parse(content);
        for (auto& item : j) {
            auto town = std::make_shared<Town>();
            town->uuid = item.value("uuid", "");
            town->name = item.value("name", "");
            town->founderUuid = item.value("founderUuid", "");
            town->founderName = item.value("founderName", "");
            town->leaderUuid = item.value("leaderUuid", "");
            for (auto& member : item.value("members", nlohmann::json::array())) {
                town->members.push_back({
                    member.value("uuid", ""),
                    member.value("name", "")
                });
            }
            towns_[town->uuid] = town;
        }
    } catch (const std::exception& e) {
        // ignore parse errors
    }
}

void DataStore::saveTowns() {
    auto path = basePath_ / "towns.json";
    nlohmann::json j = nlohmann::json::array();
    for (auto& [uuid, town] : towns_) {
        nlohmann::json item;
        item["uuid"] = town->uuid;
        item["name"] = town->name;
        item["founderUuid"] = town->founderUuid;
        item["founderName"] = town->founderName;
        item["leaderUuid"] = town->leaderUuid;
        nlohmann::json members = nlohmann::json::array();
        for (auto& m : town->members) {
            nlohmann::json member;
            member["uuid"] = m.uuid;
            member["name"] = m.name;
            members.push_back(member);
        }
        item["members"] = members;
        j.push_back(item);
    }
    saveJsonFile(path, j.dump(2));
}

void DataStore::loadNations() {
    auto path = basePath_ / "nations.json";
    auto content = loadJsonFile(path);
    if (content.empty()) return;
    try {
        nlohmann::json j = nlohmann::json::parse(content);
        for (auto& item : j) {
            auto nation = std::make_shared<Nation>();
            nation->uuid = item.value("uuid", "");
            nation->name = item.value("name", "");
            nation->founderUuid = item.value("founderUuid", "");
            nation->founderName = item.value("founderName", "");
            nation->capitalTownUuid = item.value("capitalTownUuid", "");
            for (auto& t : item.value("towns", nlohmann::json::array())) {
                nation->towns.push_back({
                    t.value("uuid", ""),
                    t.value("name", "")
                });
            }
            nations_[nation->uuid] = nation;
        }
    } catch (const std::exception& e) {
        // ignore parse errors
    }
}

void DataStore::saveNations() {
    auto path = basePath_ / "nations.json";
    nlohmann::json j = nlohmann::json::array();
    for (auto& [uuid, nation] : nations_) {
        nlohmann::json item;
        item["uuid"] = nation->uuid;
        item["name"] = nation->name;
        item["founderUuid"] = nation->founderUuid;
        item["founderName"] = nation->founderName;
        item["capitalTownUuid"] = nation->capitalTownUuid;
        nlohmann::json towns = nlohmann::json::array();
        for (auto& t : nation->towns) {
            nlohmann::json town;
            town["uuid"] = t.uuid;
            town["name"] = t.name;
            towns.push_back(town);
        }
        item["towns"] = towns;
        j.push_back(item);
    }
    saveJsonFile(path, j.dump(2));
}

void DataStore::loadPlayers() {
    auto path = basePath_ / "players.json";
    auto content = loadJsonFile(path);
    if (content.empty()) return;
    try {
        nlohmann::json j = nlohmann::json::parse(content);
        for (auto& item : j) {
            PlayerRecord record;
            record.uuid = item.value("uuid", "");
            record.name = item.value("name", "");
            record.townUuid = item.value("townUuid", "");
            record.townName = item.value("townName", "");
            record.nationUuid = item.value("nationUuid", "");
            record.nationName = item.value("nationName", "");
            players_[record.uuid] = record;
        }
    } catch (const std::exception& e) {
        // ignore parse errors
    }
}

void DataStore::savePlayers() {
    auto path = basePath_ / "players.json";
    nlohmann::json j = nlohmann::json::array();
    for (auto& [uuid, player] : players_) {
        nlohmann::json item;
        item["uuid"] = player.uuid;
        item["name"] = player.name;
        item["townUuid"] = player.townUuid;
        item["townName"] = player.townName;
        item["nationUuid"] = player.nationUuid;
        item["nationName"] = player.nationName;
        j.push_back(item);
    }
    saveJsonFile(path, j.dump(2));
}

std::string DataStore::createTown(const std::string& name, const std::string& founderUuid, const std::string& founderName) {
    std::lock_guard<std::mutex> lock(mutex_);
    // 检查是否已有同名城镇
    for (auto& [uuid, town] : towns_) {
        if (town->name == name) return "";  // 名称已存在
    }
    auto uuid = generateUuid();
    auto town = std::make_shared<Town>(uuid, name, founderUuid, founderName);
    town->addMember(founderUuid, founderName);
    towns_[uuid] = town;
    saveTowns();

    // 自动注册玩家
    registerPlayer(founderUuid, founderName);
    setPlayerTown(founderUuid, uuid, name);
    return uuid;
}

bool DataStore::deleteTown(const std::string& townUuid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto townIt = towns_.find(townUuid);
    if (townIt == towns_.end()) return false;
    auto town = townIt->second;

    // 移除该城镇关联的所有玩家
    for (auto& [uuid, player] : players_) {
        if (player.townUuid == townUuid) {
            player.townUuid = "";
            player.townName = "";
            player.nationUuid = "";
            player.nationName = "";
        }
    }

    // 如果该城镇属于某个国家，从国家中移除
    for (auto& [nUuid, nation] : nations_) {
        nation->removeTown(townUuid);
    }
    saveNations();

    towns_.erase(townIt);
    saveTowns();
    savePlayers();
    return true;
}

std::shared_ptr<Town> DataStore::getTown(const std::string& townUuid) const {
    auto it = towns_.find(townUuid);
    if (it != towns_.end()) return it->second;
    return nullptr;
}

std::shared_ptr<Town> DataStore::getTownByName(const std::string& name) const {
    for (auto& [uuid, town] : towns_) {
        if (town->name == name) return town;
    }
    return nullptr;
}

std::string DataStore::createNation(const std::string& name, const std::string& founderUuid, const std::string& founderName) {
    std::lock_guard<std::mutex> lock(mutex_);
    // 检查名称是否已存在
    for (auto& [uuid, nation] : nations_) {
        if (nation->name == name) return "";
    }
    auto uuid = generateUuid();
    auto nation = std::make_shared<Nation>(uuid, name, founderUuid, founderName);
    nations_[uuid] = nation;
    saveNations();

    // 自动注册玩家
    registerPlayer(founderUuid, founderName);
    setPlayerNation(founderUuid, uuid, name);
    return uuid;
}

bool DataStore::deleteNation(const std::string& nationUuid) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto nationIt = nations_.find(nationUuid);
    if (nationIt == nations_.end()) return false;

    // 清除该国家关联的所有玩家
    for (auto& [uuid, player] : players_) {
        if (player.nationUuid == nationUuid) {
            player.nationUuid = "";
            player.nationName = "";
        }
    }
    nations_.erase(nationIt);
    saveNations();
    savePlayers();
    return true;
}

std::shared_ptr<Nation> DataStore::getNation(const std::string& nationUuid) const {
    auto it = nations_.find(nationUuid);
    if (it != nations_.end()) return it->second;
    return nullptr;
}

void DataStore::registerPlayer(const std::string& uuid, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.find(uuid) == players_.end()) {
        players_[uuid] = PlayerRecord(uuid, name);
    }
}

PlayerRecord& DataStore::getPlayerRecord(const std::string& uuid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.find(uuid) == players_.end()) {
        players_[uuid] = PlayerRecord(uuid, "");
    }
    return players_[uuid];
}

const PlayerRecord* DataStore::getPlayerRecord(const std::string& uuid) const {
    auto it = players_.find(uuid);
    if (it != players_.end()) return &it->second;
    return nullptr;
}

void DataStore::setPlayerTown(const std::string& uuid, const std::string& townUuid, const std::string& townName) {
    auto& record = getPlayerRecord(uuid);
    record.townUuid = townUuid;
    record.townName = townName;
    savePlayers();
}

void DataStore::setPlayerNation(const std::string& uuid, const std::string& nationUuid, const std::string& nationName) {
    auto& record = getPlayerRecord(uuid);
    record.nationUuid = nationUuid;
    record.nationName = nationName;
    savePlayers();
}

std::string DataStore::getTownName(const std::string& townUuid) const {
    auto town = getTown(townUuid);
    if (town) return town->name;
    return "无城镇";
}

std::string DataStore::getNationName(const std::string& nationUuid) const {
    auto nation = getNation(nationUuid);
    if (nation) return nation->name;
    return "无国家";
}

} // namespace Towny::data
