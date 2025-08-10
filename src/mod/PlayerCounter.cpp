#include "mod/PlayerCounter.h"

#include <thread>
#include <atomic>

// LLAPI and MC Headers
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h> // FIX: Added vanilla Player header as per your suggestion

// GMLIB Headers
#include "gmlib/mc/world/actor/OfflinePlayer.h"
#include "gmlib/mc/world/actor/Player.h"

namespace player_counter {

bool PlayerCounterMod::load() {
    mLogger.info("PlayerCounter Mod 正在加载...");
    return true;
}

bool PlayerCounterMod::enable() {
    mLogger.info("PlayerCounter Mod 正在启用...");
    registerCommand();
    return true;
}

bool PlayerCounterMod::disable() {
    mLogger.info("PlayerCounter Mod 正在禁用...");
    return true;
}

void PlayerCounterMod::registerCommand() {
    using namespace ll::command;

    auto& command =
        CommandRegistrar::getInstance()
            .getOrCreateCommand(
                "playercount",
                "获取服务器存档中的总玩家数量",
                CommandPermissionLevel::Admin // Use 'Admin' for operator permissions
            );

    command.overload<>(
        [this](CommandOrigin const& origin, CommandOutput& output) {
            mce::UUID playerUuid = origin.getPlayer() ? origin.getPlayer()->getUUID() : mce::UUID::EMPTY;

            output.success("§a正在后台统计玩家数量，请稍候...");

            std::thread([this, playerUuid]() {
                std::atomic<int> count = 0;

                gmlib::OfflinePlayer::forEachOfflinePlayer([&](gmlib::OfflinePlayer&& player) {
                    count++;
                    return true; // Continue iterating
                });

                // If the command was run from the console (empty UUID)
                if (playerUuid.isEmpty()) {
                    mLogger.info("服务器总计拥有 {} 名玩家的存档数据。", count.load());
                    return;
                }

                // If run by a player, try to find them back to send the result.
                auto onlinePlayers = ll::service::getOnlinePlayers();
                for (const auto& p : onlinePlayers) {
                    if (p->getUUID() == playerUuid) {
                        // Player found, cast to GMPlayer to use GMLIB's sendText.
                        auto* gmPlayer = static_cast<gmlib::GMPlayer*>(p.get());
                        gmPlayer->sendText(
                            "§b[PlayerCounter] §e服务器总计拥有 §f" + std::to_string(count) + " §e名玩家的存档数据。"
                        );
                        return; // Task finished
                    }
                }

                // Fallback if the player logged off before the task finished.
                mLogger.info("玩家 {} 的统计任务完成，结果: {}。该玩家已离线。", playerUuid.asString(), count.load());

            }).detach();
        }
    );
}

} // namespace player_counter
