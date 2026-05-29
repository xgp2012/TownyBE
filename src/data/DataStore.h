#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <mutex>
#include <memory>
#include "Town.h"
#include "Nation.h"

namespace Towny::data {

struct PlayerRecord {
    std::string uuid;
    std::string name;
    std::string townUuid;  // 空字符串表示无城镇
    std::string townName;  // 城镇名称缓存
    std::string nationUuid;  // 空字符串表示无国家
    std::string nationName;  // 国家名称缓存

    PlayerRecord() = default;
    PlayerRecord(std::string u, std::string n)
        : uuid(std::move(u)), name(std::move(n)) {}
};

class DataStore {
public:
    static DataStore& getInstance();

    // 初始化和持久化
    void init(const std::filesystem::path& basePath);
    void save();

    // 城镇操作
    std::string createTown(const std::string& name, const std::string& founderUuid, const std::string& founderName);
    bool deleteTown(const std::string& townUuid);
    std::shared_ptr<Town> getTown(const std::string& townUuid) const;
    std::shared_ptr<Town> getTownByName(const std::string& name) const;
    const std::unordered_map<std::string, std::shared_ptr<Town>>& getAllTowns() const { return towns_; }

    // 国家操作
    std::string createNation(const std::string& name, const std::string& founderUuid, const std::string& founderName);
    bool deleteNation(const std::string& nationUuid);
    std::shared_ptr<Nation> getNation(const std::string& nationUuid) const;
    const std::unordered_map<std::string, std::shared_ptr<Nation>>& getAllNations() const { return nations_; }

    // 玩家操作
    void registerPlayer(const std::string& uuid, const std::string& name);
    PlayerRecord& getPlayerRecord(const std::string& uuid);
    const PlayerRecord* getPlayerRecord(const std::string& uuid) const;

    // 设置玩家所属
    void setPlayerTown(const std::string& uuid, const std::string& townUuid, const std::string& townName);
    void setPlayerNation(const std::string& uuid, const std::string& nationUuid, const std::string& nationName);

    // 从 UUID 获取名称
    std::string getTownName(const std::string& townUuid) const;
    std::string getNationName(const std::string& nationUuid) const;

private:
    DataStore() = default;
    ~DataStore() = default;

    void loadTowns();
    void saveTowns();
    void loadNations();
    void saveNations();
    void loadPlayers();
    void savePlayers();

    std::filesystem::path basePath_;
    std::unordered_map<std::string, std::shared_ptr<Town>> towns_;
    std::unordered_map<std::string, std::shared_ptr<Nation>> nations_;
    std::unordered_map<std::string, PlayerRecord> players_;
    mutable std::mutex mutex_;
};

} // namespace Towny::data
