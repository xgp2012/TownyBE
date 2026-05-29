#pragma once

namespace ll::command {
class CommandRegistrar;
}
namespace mc {
class Player;
}
namespace Towny::commands {

void registerTownCommands();
void registerNationCommands();

void handleCreateTown(mc::Player& player, const std::string& townName);
void handleLeaveTown(mc::Player& player);
void handleInviteMember(mc::Player& player, const std::string& targetName);
void handleSetLeader(mc::Player& player, const std::string& targetName);
void handleDisbandTown(mc::Player& player);

void handleCreateNation(mc::Player& player, const std::string& nationName);
void handleDisbandNation(mc::Player& player);
void handleAddTown(mc::Player& player, const std::string& townName);
void handleRemoveTown(mc::Player& player, const std::string& townName);

} // namespace Towny::commands
